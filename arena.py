import math
import processed_marker
import numpy as np
import cv2

width = 4.0 #width and heignt of the arena in meters
height = 2.0
#m_list = []
def getHomographyMatrix(frame,marker_list):
    #print("doing homogrphy")
    pt00 = (-1,1)
    pt02 = (-1,1)
    pt40 = (-1,1)
    pt42 = (-1,1) #some dummy values
    first = False
    for x in marker_list:
        #print(f'id {x.id} = {x.corner1}')
        if x.id == 0:  #finding all the corners of the arena
            pt00 = x.corner1
        elif x.id == 1:
            pt40 = x.corner1
        elif x.id == 2:
            pt02 = x.corner1
        elif x.id == 3:
            pt42 = x.corner1
    
    #print('\n')
    if -1 in pt00 or -1 in pt02 or -1 in pt40 or -1 in pt42:
        first = True
    src_pts = np.float32([pt00, pt40, pt02, pt42]) #pixel coordinates of the markers
    
    dst_pts = np.float32([[0.0, 0.0], [width, 0.0], [0.0, height], [width, height]]) #arena coordinates of markers
    H = cv2.getPerspectiveTransform(src_pts, dst_pts) #this gives us the H matrix of the arena
    #print(H.shape)
    #print("homography done")
    return H, first

def processMarkers(frame, marker_list, H,inverse_matrix, dr_op):
    #print("processing markers")
    markers = {}
    markers[f'{0}'] = processed_marker.processed_Marker(0, -1.0, -1.0, -1.0)
    markers[f'{1}'] = processed_marker.processed_Marker(1, -1.0, -1.0, -1.0)
    markers[f'{2}'] = processed_marker.processed_Marker(2, -1.0, -1.0, -1.0)
    markers[f'{3}'] = processed_marker.processed_Marker(3, -1.0, -1.0, -1.0)
    for x in marker_list:
        if x.id > 3:
            n_marker = translate(x, H)
            
            markers[f'{n_marker.id}'] = n_marker #adding in the processed marker to a dictionary
            #print(x.id)
            #m_list.append(n_marker)
            
            #Add a green arrowed line to each aruco marker.
            #Note, opencv does not give a way to change the id color
            frame = cv2.arrowedLine(frame,(int(x.corner1[0]), int(x.corner1[1])), (int(x.corner2[0]), 
                                    int(x.corner2[1])), (0, 255, 0), 2, tipLength=.4)
    #draw the obstacles if we want to        
    if dr_op.draw_obstacles:
        frame = createObstacles(frame,inverse_matrix,dr_op.randomization)
   #draw the mission if we want to
    if dr_op.draw_dest:
        frame = createMission(frame,inverse_matrix,dr_op.otv_start_dir,dr_op.mission_loc,dr_op.otv_start_loc)
    #returned the processed image frame and marker list
    return frame, markers

def createMission(frame, inverse_matrix,theta, mission_loc, start_loc) :
    y = [.55,1.45] #possible y coordinates of the mission and otv
    #inverse_matrix = np.linalg.pinv(H) #find the inverse matrix of the homography matrix
    red = (25,25,215) #Note, opencv does colors in (B,G,R)
    white = (255,255,255)
    


    #draws the mission site
    point1 = np.float32(np.array([[[0.575, y[mission_loc]]]]))
    transformed_1 = cv2.perspectiveTransform(point1, inverse_matrix)
    frame = cv2.circle(frame,(int(transformed_1[0,0,0]),int(transformed_1[0,0,1])),20,red,2)
    
    
    #finding the coordinates of two points of arrowed line
    #the start of the arrow
    x_c = .25 * math.cos(theta) + 0.575 #using 0.575 instead of 0.55 to accound for camera angle
    y_c = .25 * math.sin(theta) + y[start_loc]
    #the tip of the arrow
    x_s = .125 * math.cos(theta - math.pi) + 0.575 
    y_s = .125 * math.sin(theta - math.pi) + y[start_loc]
    
    #drawing the arrowed line
    point1 = np.float32(np.array([[[x_s,y_s]]]))
    point2 = np.float32(np.array([[[x_c, y_c]]]))
    transformed_1 = cv2.perspectiveTransform(point1, inverse_matrix)
    transformed_2 = cv2.perspectiveTransform(point2, inverse_matrix)
    frame = cv2.arrowedLine(frame,(int(transformed_1[0,0,0]),int(transformed_1[0,0,1])),
            (int(transformed_2[0,0,0]),int(transformed_2[0,0,1])),white,3)\
    
    #drawing the otv box, x-coordinate will always be .55 as the center
    point1 = np.float32(np.array([[[0.55 - 0.25, y[start_loc] - 0.25]]])) #bottom left corner of square
    point2 = np.float32(np.array([[[0.55 + 0.25, y[start_loc] + 0.25]]])) #top right corner of square
    transformed_1 = cv2.perspectiveTransform(point1, inverse_matrix)
    transformed_2 = cv2.perspectiveTransform(point2, inverse_matrix)
    frame = cv2.rectangle(frame,(int(transformed_1[0,0,0]),int(transformed_1[0,0,1])),
            (int(transformed_2[0,0,0]),int(transformed_2[0,0,1])),white,3)

    return frame

def createObstacles(frame,inverse_matrix, instruction):
    possible_x = [1.35, 2.2] # possible x-coords of obstacles
    possible_y = [1.25,0.75,0.25] # possible y-coords of obstacles, in decreasing order due to randomization
    rows = [0,1,2] #keeps track of which rows have obstacles filled by removing that row from the list
    x_length = 0.2 # equiv to 20cm
    y_length = 0.5 # equiv to 50cm 
    blue = (185,146,68) # color of solid obstacle
    gold = (25,177,215) # color of traversable obstacle
    #inverse_matrix = np.linalg.pinv(H) # inverts the homography matrix so we can convert arena coords to pixel coords
    
    #draw out the solid obstacles
    for x in range(2):
        placement = int(instruction[x])    
        point1 = np.float32(np.array([[[possible_x[x], possible_y[placement]]]])) #bottom left corner
        point2 = np.float32(np.array([[[possible_x[x] + x_length, possible_y[placement] + y_length]]])) #top right corner
        point3 = np.float32(np.array([[[possible_x[x] + 0.05, possible_y[placement] + 0.25]]])) #text poisition
        
        rows.remove(placement) #removed processed markers, probably a more efficient way to do this but it works
        
        #transforms arena coordinates to pixel coordinates
        transformed_1 = cv2.perspectiveTransform(point1, inverse_matrix) 
        transformed_2 = cv2.perspectiveTransform(point2, inverse_matrix)
        text = cv2.perspectiveTransform(point3, inverse_matrix)

        # tranformed will give a float array, got to cast to int for this to work properly
        frame = cv2.rectangle(frame,(int(transformed_1[0,0,0]),int(transformed_1[0,0,1])),
            (int(transformed_2[0,0,0]),int(transformed_2[0,0,1])),blue,3)
        frame = cv2.putText(frame, 'S', (int(text[0,0,0]),int(text[0,0,1])), cv2.FONT_HERSHEY_SIMPLEX, 
                   1, (255,0,0), 2, cv2.LINE_AA)
    
    #drawing out the traversable object    
    placement = instruction[2]
    if placement == "A":
        point1 = np.float32(np.array([[[possible_x[0], possible_y[rows[0]]]]]))
        point2 = np.float32(np.array([[[possible_x[0] + x_length, possible_y[rows[0]] + y_length]]]))
        point3 = np.float32(np.array([[[possible_x[0] + 0.05, possible_y[rows[0]] + 0.25]]]))
    else:
        point1 = np.float32(np.array([[[possible_x[1], possible_y[rows[0]]]]]))
        point2 = np.float32(np.array([[[possible_x[1] + x_length, possible_y[rows[0]] + y_length]]]))
        point3 = np.float32(np.array([[[possible_x[1] + 0.05, possible_y[rows[0]] + 0.25]]]))
    
    transformed_1 = cv2.perspectiveTransform(point1, inverse_matrix)
    transformed_2 = cv2.perspectiveTransform(point2, inverse_matrix)
    
    text = cv2.perspectiveTransform(point3, inverse_matrix)

    # tranformed will give a float array, got to cast to int for this to work properly
    frame = cv2.rectangle(frame,(int(transformed_1[0,0,0]),int(transformed_1[0,0,1])),
        (int(transformed_2[0,0,0]),int(transformed_2[0,0,1])),gold,3)
    frame = cv2.putText(frame, 'T', (int(text[0,0,0]),int(text[0,0,1])), cv2.FONT_HERSHEY_SIMPLEX, 
                   1, (255,0,0), 2, cv2.LINE_AA)
    
    return frame  

def translate(marker, H):
    #print("translating")
    # find the center of the marker in pixels
    marker_coords_px = np.float32(np.array([[[0.0, 0.0]]]))  # dont know why you need so many brakets, but this makes it work
    marker_coords_px[0, 0, 0] = (marker.corner1[0] + marker.corner2[0] + marker.corner3[0] + marker.corner4[0]) / 4
    marker_coords_px[0, 0, 1] = (marker.corner1[1] + marker.corner2[1] + marker.corner3[1] + marker.corner4[1]) / 4

    # Use homography transformation matrix to convert marker coords in px to meters
    marker_coords_m = cv2.perspectiveTransform(marker_coords_px, H)[0]
    #print(marker_coords_m)

    # Find theta of the marker
    corner1_coords_m = cv2.perspectiveTransform(np.float32(np.array([[marker.corner1]])), H)
    corner2_coords_m = cv2.perspectiveTransform(np.float32(np.array([[marker.corner2]])), H)
    marker_theta = math.atan2(corner2_coords_m[0, 0, 1] - corner1_coords_m[0, 0, 1], corner2_coords_m[0, 0, 0] - corner1_coords_m[0, 0, 0])
    #print(marker_theta)
    n_marker = processed_marker.processed_Marker(marker.id, marker_coords_m[0,0], marker_coords_m[0,1], marker_theta)

    #print("translation done")
    return n_marker
