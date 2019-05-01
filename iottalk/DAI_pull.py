import time, DAN, requests, random
import json
import numpy as np
import cv2
import base64
from ast import literal_eval

import ast

#from requests.utils import requote_uri


#ServerURL = 'http://IP:9999' #with no secure connection
#ServerURL = 'https://DomainName' #with SSL connection
ServerURL = 'http://140.113.86.143:9999'
Reg_addr = None #if None, Reg_addr = MAC address

#DAN.profile['dm_name']='HumanRecognition'
#DAN.profile['df_list']=['BoxCoord-O', 'BoxID-O', 'FrameID-O']
DAN.profile['dm_name']='HumanRecognition2'
DAN.profile['df_list']=['BoxCoordsX-O', 'BoxCoordsY-O', 'BoxIDs-O', 'FrameID-O', 'UserCoordsX-O', 'UserCoordsY-O', 'UserIDs-O', 'DisplayIDs-O']
DAN.profile['d_name']= None # None for autoNaming
DAN.device_registration_with_retry(ServerURL, Reg_addr)

#cap = cv2.VideoCapture('time_counter.flv')

all_beacon_information_record = None

def receive_frame_from_iottalk():

    global all_beacon_information_record

    start_time = time.time()
    time.sleep(0.02)

    #print('start receive frame from iottalk')
    all_boxes_information = list()
    all_beacon_information = list()
    display_ids = list()
    display_ids_record = None
    try:

        # pull boxes information
        pull_box = False
        box_x = DAN.pull('BoxCoordsX-O')
        box_y = DAN.pull('BoxCoordsY-O')
        box_ids = DAN.pull('BoxIDs-O')
        frame_id = DAN.pull('FrameID-O')

        #print(box_x, box_y, box_ids, frame_id)

        # pull beacon information
        user_x = DAN.pull('UserCoordsX-O')
        user_y = DAN.pull('UserCoordsY-O')
        user_ids = DAN.pull('UserIDs-O')

        # pull display information
        display_ids = DAN.pull('DisplayIDs-O')


        # boxes information
        if box_x != None and box_y != None and box_ids != None and frame_id != None:

            print("pull boxes information")
            #print(box_ids)
            #print(box_x)
            #print(box_y)

            box_x = ast.literal_eval(box_x[0])
            box_y = ast.literal_eval(box_y[0])
            box_ids = ast.literal_eval(box_ids[0])
            frame_id = frame_id[0]

            #print('frame: {}'.format(frame_id))

            for i in range(len(box_ids)):
                #print('person: {}'.format(box_ids[i]))
                #print('x: {}, {}'.format(box_x[i*2], box_x[i*2+1]))
                #print('y: {}, {}'.format(box_y[i*2], box_y[i*2+1]))

                boxes_information = dict()
                boxes_information['stamp'] = frame_id
                boxes_information['id'] = box_ids[i]
                boxes_information['x1'] = box_x[i*2]
                boxes_information['y1'] = box_y[i*2]
                boxes_information['x2'] = box_x[i*2+1]
                boxes_information['y2'] = box_y[i*2+1]

                all_boxes_information.append(boxes_information)

            #print(all_boxes_information)
            pull_box = True

        # beacon information
        if pull_box and user_x != None and user_y != None and user_ids != None:

            print("pull beacon information")
            #print(user_x)
            #print(user_y)
            #print(user_ids)

            user_x = ast.literal_eval(user_x[0])
            user_y = ast.literal_eval(user_y[0])
            user_ids = ast.literal_eval(user_ids[0])

            for i in range(len(user_ids)):
                #print('person: {}'.format(user_ids[i]))
                #print('x: {}'.format(user_x[i]))
                #print('y: {}'.format(user_y[i]))

                beacon_information = dict()
                beacon_information['id'] = user_ids[i]
                beacon_information['x'] = user_x[i]
                beacon_information['y'] = user_y[i]

                all_beacon_information.append(beacon_information)

            all_beacon_information_record = all_beacon_information

        # display information
        if display_ids != None:

            #print("pull display information")
            #print(display_ids)

            display_ids = ast.literal_eval(display_ids[0])
            display_ids_record = list(display_ids)

            #print(display_ids)
        #display_ids_record = [1, 2]

        #time.sleep(0.02)
        print((all_boxes_information, display_ids_record, all_beacon_information_record), time.time() - start_time)
        return (all_boxes_information, display_ids_record, all_beacon_information_record)
            #return (tmp_boxes, tmp_enable_name, tmp_beacon)

            
            #print(frame_id)
            #print(box_coord)
            #print(box_id)

            #print(box_coord)
            
            #box_coords = json.loads(box_coord[0])
            #print(str(frame_id[0]) + ' : ' + str(box_coords))

            #boxes_information['stamp'] = frame_id[0]
            #boxes_information['id'] = box_id[0]
            #boxes_information['x1'] = box_coord[0]
            #boxes_information['y1'] = box_coord[1]
            #boxes_information['x2'] = box_coord[2]
            #boxes_information['y2'] = box_coord[3]
            #print(boxes_information)

            #all_boxes_information = data[0].split('|')
            #all_boxes_information_size = len(all_boxes_information)
            #boxes_information = all_boxes_information[0]
            #enable_name = all_boxes_information[all_boxes_information_size - 1]
            #print(person_information)
            
            
            
            #tmp_boxes = json.loads(boxes_information)
            #tmp_enable_name = json.loads(enable_name)
            #print(tmp_boxes)
            #print(tmp_enable_name)

            #tmp_beacon = list()
            #for i in range(1, all_boxes_information_size - 1):
            #    tmp_beacon.append(json.loads(all_boxes_information[i]))

            #print(tmp_beacon)
            
            #tmp_nparray = np.array(tmp_array)
            #tmp_buf = tmp_nparray.astype('uint8')
            #frame = cv2.imdecode(tmp_buf, 1)
            #cv2.imshow('Receive',frame)
            #cv2.waitKey(1)

            #return (tmp_boxes, tmp_enable_name, tmp_beacon)

    except Exception as e:
        print(e)
        if str(e).find('mac_addr not found:') != -1:
            print('Reg_addr is not found. Try to re-register...')
            DAN.device_registration_with_retry(ServerURL, Reg_addr)
        else:
            print('Connection failed due to unknow reasons.')
            #time.sleep(1)    

    #time.sleep(0.02)
    return None


if __name__ == "__main__":

    while True:
        receive_frame_from_iottalk()

'''def receive_frame_from_iottalk():

    #print('start receive frame from iottalk')
    try:
        data = DAN.pull('ODF_ALL')
        if data != None:
            print("pull")
            print(data[0])
            all_boxes_information = data[0].split('|')
            all_boxes_information_size = len(all_boxes_information)
            boxes_information = all_boxes_information[0]
            enable_name = all_boxes_information[all_boxes_information_size - 1]
            #print(person_information)
            tmp_boxes = json.loads(boxes_information)
            tmp_enable_name = json.loads(enable_name)
            print(tmp_boxes)
            print(tmp_enable_name)

            tmp_beacon = list()
            for i in range(1, all_boxes_information_size - 1):
                tmp_beacon.append(json.loads(all_boxes_information[i]))

            print(tmp_beacon)
            
            #tmp_nparray = np.array(tmp_array)
            #tmp_buf = tmp_nparray.astype('uint8')
            #frame = cv2.imdecode(tmp_buf, 1)
            #cv2.imshow('Receive',frame)
            #cv2.waitKey(1)

            return (tmp_boxes, tmp_enable_name, tmp_beacon)

    except Exception as e:
        print(e)
        if str(e).find('mac_addr not found:') != -1:
            print('Reg_addr is not found. Try to re-register...')
            DAN.device_registration_with_retry(ServerURL, Reg_addr)
        else:
            print('Connection failed due to unknow reasons.')
            #time.sleep(1)    

    #time.sleep(0.2)
    return None'''
