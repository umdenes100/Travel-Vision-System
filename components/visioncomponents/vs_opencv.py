import json
import logging
import os.path
import sys
import threading

import components.communications.client_server
from components.communications.client_server import send_frame
from components.communications.esp_server import send_locations

if 'local' not in sys.argv:
    import cv2
    import numpy as np

from components.data import dr_op, camera
from components.visioncomponents.vs_arena import getHomographyMatrix, processMarkers, createObstacles, createMission
import time

config = {}
if os.path.isfile(os.path.expanduser('~/config.json')):
    with open(os.path.expanduser('~/config.json')) as f:
        config = json.load(f)

dictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_1000)
parameters = cv2.aruco.DetectorParameters()
detector = cv2.aruco.ArucoDetector(dictionary, parameters)
camera_bounds = None


def corners_found(ids):
    return len(ids) >= 4 and all([i in ids for i in range(4)])


def render_issue(issue: str, frame):
    components.communications.client_server.send_console_message(issue)
    for y in range(50, frame.shape[0], 50):
        frame = cv2.putText(frame, issue, (50, y), cv2.FONT_HERSHEY_TRIPLEX, 2, (0, 0, 255))

    return frame


def process_frame(frame):
    try:
        (corners, ids, rejected) = detector.detectMarkers(frame)
        frame = cv2.aruco.drawDetectedMarkers(frame, corners)

        # Serious ish error.
        if ids is None or corners is None:
            if dr_op.camera_matrix is not None:
                frame = cv2.warpPerspective(frame, dr_op.camera_matrix, (frame.shape[1], frame.shape[0]))
            return frame

        ids = [i[0] for i in ids]
        corners = [c[0] for c in corners]
        # logging.debug(f'corners {corners}')
        # logging.debug(f'ids {ids}')

        if dr_op.H is None:
            if not (isinstance(ids, list) or isinstance(ids, np.ndarray)):
                return render_issue('No corners were found', frame)
            if not corners_found(ids):
                return render_issue('One of the markers is blocked - cannot generate homography matrix', frame)

            dr_op.H, dr_op.camera_matrix = getHomographyMatrix(zip(ids, corners),
                                                               camera_width=frame.shape[1],
                                                               camera_height=frame.shape[0])
            if dr_op.H is None:
                return render_issue('At least one of the corner ArUco markers are not visible - homography matrix could not be generated', frame)
            components.communications.client_server.send_console_message('Initialized Homography Matrix (All corners visible)')
            dr_op.inverse_matrix = np.linalg.pinv(dr_op.H)

        if dr_op.draw_obstacles:
            frame = createObstacles(frame, dr_op.inverse_matrix, dr_op.randomization)

        if dr_op.draw_dest:
            frame = createMission(frame, dr_op.inverse_matrix, dr_op.otv_start_dir, dr_op.mission_loc,
                                  dr_op.otv_start_loc)

        # Warp the frame here. From now on, you must multiply points by the camera matrix to get stuff to line up.
        frame = cv2.warpPerspective(frame, dr_op.camera_matrix, (frame.shape[1], frame.shape[0]))

        dr_op.aruco_markers = processMarkers(zip(ids, corners))
        for marker in dr_op.aruco_markers.values():
            # if marker.id not in range(3):
            # print('marker:', marker)
            try:
                original = np.array([marker.pixels[0], marker.pixels[1], 1])
                warped = np.dot(dr_op.camera_matrix, original)
                warped[0] /= warped[2]
                warped[1] /= warped[2]
                pixels = (int(warped[0]), int(warped[1]))
                # print('Original: ', original)
                # print('Warped: ', warped)
                frame = cv2.putText(frame, 'ID:' + str(marker.id), pixels, cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0))
            except Exception as e:
                print('exception when processing marker' + str(marker), str(marker.pixels), e)
                import traceback
                print(traceback.format_exc())
        def tuple_int(x):
            return tuple(map(int, x))

        for i, marker in zip(ids, corners):
            if i not in range(4):
                corner1 = tuple_int(marker[0])
                corner2 = tuple_int(marker[1])
                originalc1 = np.array([corner1[0], corner1[1], 1])
                originalc2 = np.array([corner2[0], corner2[1], 1])
                warpedc1 = np.dot(dr_op.camera_matrix, originalc1)
                warpedc2 = np.dot(dr_op.camera_matrix, originalc2)
                warpedc1[0] /= warpedc1[2]
                warpedc1[1] /= warpedc1[2]
                warpedc2[0] /= warpedc2[2]
                warpedc2[1] /= warpedc2[2]
                pixels1 = (int(warpedc1[0]), int(warpedc1[1]))
                pixels2 = (int(warpedc2[0]), int(warpedc2[1]))
                frame = cv2.arrowedLine(frame, pixels1, pixels2, (0, 255, 0), 2,
                                        tipLength=.4)

    except KeyboardInterrupt:
        exit()
    except Exception as e:
        logging.debug(str(e))
        import traceback
        logging.debug(traceback.format_exc())
    return frame


target_fps = 20
last_sleep = time.perf_counter()

img_info = {
    'bytes_sent': 0,
    'frames_sent': 0,
}


def start_image_processing():
    print_fps_time = time.perf_counter()
    send_locations_bool = True  # send locations to esp32 every other frame
    while True:
        # If a web client is connected
        # if len(communications.client_server.ws_server.clients) == 0 and len(communications.esp_server.ws_server.clients) == 0:
        #     logging.debug('No clients connected, slowing image processing to 1fps')
        #     time.sleep(1)
        start = time.perf_counter()  # start timer to calculate FPS
        try:
            cap = camera.get_camera()  # get video stream from connections object
            if cap.isOpened():
                ret, frame = cap.read()  # read frame from video stream
                if ret:
                    new_frame = process_frame(frame)
                    if send_locations_bool:  # send locations to esp32 every other frame
                        threading.Thread(target=send_locations, name='Send Locations').start()
                    send_locations_bool = not send_locations_bool

                    jpeg_bytes = cv2.imencode('.jpg', new_frame, [int(cv2.IMWRITE_JPEG_QUALITY), 30])[1]
                    img_info['bytes_sent'] += len(jpeg_bytes)
                    img_info['frames_sent'] += 1
                    # logging.log(f'Image size: {len(jpeg_bytes)} bytes')
                    send_frame(np.array(jpeg_bytes).tostring())  # send frame to web client
                else:
                    time.sleep(1)
                    logging.debug('Could not read frame from camera, restarting stream')
                    time.sleep(1)
                    camera.restart_stream()
            else:
                time.sleep(1)
                logging.debug('Camera is not open, restarting stream')
                time.sleep(1)
                camera.restart_stream()
        except Exception as e:
            logging.debug(str(e))
        global last_sleep
        sleep_time = (1 / target_fps) - (time.perf_counter() - last_sleep)
        if sleep_time > 0:
            time.sleep(sleep_time)
        last_sleep = time.perf_counter()

        if time.perf_counter() - print_fps_time > 10:
            print_fps_time = time.perf_counter()
            try:
                logging.debug(
                    f'{1 / (time.perf_counter() - start):.2f} fps - avg {img_info["bytes_sent"] / img_info["frames_sent"] / 1000:.0f} kb per frame')  # print FPS
            except ZeroDivisionError:
                pass
        img_info['bytes_sent'] = 0
        img_info['frames_sent'] = 0
