import PySimpleGUI as sg
from enum import Enum
import argparse
from datetime import datetime, timedelta
import socket
import threading

SERVER_IP = 'localhost'
SERVER_PORT = 2137
CLIENT_IP = ''
CLIENT_PORT = 8080

NAME_MAX_LENGTH = 64
ALIAS_MAX_LENGTH = 32
TIMEOUT = 3
BUF_SIZE = 256

#  @TODO: broken pipe error handling?

#  @TODO handle '\0' sign?????
#  @TODO mutex for printing?

#  @TODO test connection for multiple clients

#  @TODO  size of message - block or send it multiple times to server if message is long?

# @TODO demon???


class client:

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum):
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1

    _quit = 0

    _username = None
    _alias = None
    _date = None

    _client_ip = ''
    _client_port = None

    _socket = None
    _connection_thread = None
    _disconnect = threading.Event()

    # ******************** METHODS *******************
    # *
    # * @param user - User name to register in the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user is already registered
    # * @return ERROR if another error occurred
    @staticmethod
    def register(user, window):
        """
        function performs registering user from the client side
        it sends username, alias and date of birth to server
        and waits fot the response if registartion went right
        """

        # preparing data
        date = client._date.replace("-", "/")

        # creating socket
        s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)

        # connect to server
        s.connect((client._server, client._port))

        # send data one by one
        s.send(bytes("REGISTER\0", 'UTF-8'))
        s.send(bytes(client._username + "\0", 'UTF-8'))
        s.send(bytes(client._alias + "\0", 'UTF-8'))
        s.send(bytes(date + "\0", 'UTF-8'))

        # receive response with set timeout from server and close connection
        s.settimeout(TIMEOUT)
        try:
            response = int(s.recv(1).decode())
        except socket.timeout:
            # Handle a timeout exception
            sg.Popup(f'Timeout occured, no data received within {TIMEOUT} sec', title='TIMEOUT', button_type=5, auto_close=True, auto_close_duration=3)
            response = 2
        s.close()

        # get and interpret the response
        match response:
            case 0:
                window['_SERVER_'].print("s> REGISTER OK")
                return client.RC.OK
            case 1:
                window['_SERVER_'].print("s> USERNAME IN USE")
                return client.RC.USER_ERROR
            # not only for case 2 but for any other
            case _:
                window['_SERVER_'].print("s> REGISTER FAIL")
                return client.RC.ERROR

    # *
    # 	 * @param user - User name to unregister from the system
    # 	 *
    # 	 * @return OK if successful
    # 	 * @return USER_ERROR if the user does not exist
    # 	 * @return ERROR if another error occurred
    @staticmethod
    def unregister(user, window):
        """
        function performs unregistering client from the client side
        it sends operation name and alias fo user that we want to unregister
        and waits until the response if it succeed
        """

        # creating socket
        s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)

        # connect to server
        s.connect((client._server, client._port))

        # send data - operation name + alias
        s.send(bytes("UNREGISTER\0", 'UTF-8'))
        s.send(bytes(client._alias + "\0", 'UTF-8'))

        # receive response with set timeout from server and close connection
        s.settimeout(TIMEOUT)
        try:
            response = int(s.recv(1).decode())
        except socket.timeout:
            # Handle a timeout exception
            sg.Popup(f'Timeout occured, no data received within {TIMEOUT} sec', title='TIMEOUT', button_type=5, auto_close=True, auto_close_duration=3)
            response = 2
        s.close()

        # print response on the frontend
        match response:
            case 0:
                window['_SERVER_'].print("s> UNREGISTER OK")
                return client.RC.OK
            case 1:
                window['_SERVER_'].print("s> USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            # not only for case 2 but for any other
            case _:
                window['_SERVER_'].print("s> UNREGISTER FAIL")
                return client.RC.ERROR

    def start_connection(window):
        client._socket.listen()
        print('TCP client is listening')

        try:
            while True:
                conn, addr = client._socket.accept()
                if client._disconnect.is_set():
                    break
                print(f'Connected to client from host {addr[0]}, on port {addr[1]}')
                while True:
                    data = conn.recv(13+ALIAS_MAX_LENGTH+4+BUF_SIZE)  # 13 for SEND_MESSAGE, 4 for id of message
                    if len(data) == 0:
                        break
                    data_split = data.split(b'\0')
                    if len(data_split) >= 4:
                        operation, user, mess_id, message, e = data_split
                        if operation.decode() == "SEND_MESSAGE":
                            print(f'Thread id: {threading.get_native_id()}, received message: {operation} from {user}')
                            window['_SERVER_'].print(f's> MESSAGE {mess_id.decode()} FROM {user.decode()}\n     {message.decode()}\n     END')
        except socket.error:
            print("Socket has been shutted down - user disconnected")
            return

    # *
    # * @param user - User name to connect to the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist or if it is already connected
    # * @return ERROR if another error occurred

    @staticmethod
    def connect(user, window):

        # client is behaving like server here so we create socket for communication here and bind it
        client._socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
        # not to wait 1 minute until the adress is free
        client._socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # the way to find free port
        client._socket.bind((client._client_ip, 0))
        # search for the the free port that system gave us in this network
        client._client_port = client._socket.getsockname()[1]
        print(f'Client port is: {client._client_port}')
        # creating a thread but not starting it since we don't know if its a legal action without resposne from server
        client._connection_thread = threading.Thread(target=client.start_connection, args=(window, ), daemon=True)

        # creating socket
        s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)

        # connect to server
        s.connect((client._server, client._port))

        # send data - operation name + alias
        s.send(bytes("CONNECT\0", 'UTF-8'))
        s.send(bytes(client._alias + "\0", 'UTF-8'))
        s.send(bytes(str(client._client_port) + "\0", 'UTF-8'))

        # receive response with set timeout from server and close connection
        s.settimeout(TIMEOUT)
        try:
            response = int(s.recv(1).decode())
        except socket.timeout:
            # Handle a timeout exception
            sg.Popup(f'Timeout occured, no data received within {TIMEOUT} sec', title='TIMEOUT', button_type=5, auto_close=True, auto_close_duration=3)
            response = 3
        s.close()

        # print response on the frontend
        match response:
            case 0:
                # if it's ok, we are starting the thread
                client._connection_thread.start()
                window['_SERVER_'].print("s> CONNECT OK")
                return client.RC.OK
            case 1:
                window['_SERVER_'].print("s> CONNECT FAIL, USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            case 2:
                window['_SERVER_'].print("s> USER ALREADY CONNECTED")
                return client.RC.USER_ERROR
            # not only for case 3 but for any other
            case _:
                window['_SERVER_'].print("s> CONNECT FAIL")
                return client.RC.ERROR

    # *
    # * @param user - User name to disconnect from the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist
    # * @return ERROR if another error occurred

    @staticmethod
    def disconnect(user, window):

        # creating socket
        s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)

        # connect to server
        s.connect((client._server, client._port))

        # send data - operation name + alias
        s.send(bytes("DISCONNECT\0", 'UTF-8'))
        s.send(bytes(client._alias + "\0", 'UTF-8'))

        # receive response from server and close connection
        s.settimeout(TIMEOUT)
        # receive response with set timeout from server and close connection
        try:
            response = int(s.recv(1).decode())
        except socket.timeout:
            # Handle a timeout exception
            sg.Popup(f'Timeout occured, no data received within {TIMEOUT} sec', title='TIMEOUT', button_type=5, auto_close=True, auto_close_duration=3)
            response = 3
        s.close()

        # print response on the frontend
        match response:
            case 0:
                client._socket.shutdown(socket.SHUT_RD)
                client._socket.close()
                client._disconnect.set()
                client._connection_thread.join()
                client._socket = None
                client._client_port = None
                window['_SERVER_'].print("s> DISCONNECT OK")
                return client.RC.OK
            case 1:
                window['_SERVER_'].print("s> DISCONNECT FAIL / USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            case 2:
                window['_SERVER_'].print("s> DISCONNECT FAIL / USER NOT CONNECTED")
                return client.RC.USER_ERROR
            # not only for case 3 but for any other
            case _:
                window['_SERVER_'].print("s> DISCONNECT FAIL")
                return client.RC.ERROR

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def send(user, message, window):
        # creating socket
        s = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)

        # connect to server
        s.connect((client._server, client._port))

        # send data - operation name + alias
        s.send(bytes("SEND\0", 'UTF-8'))
        s.send(bytes(client._alias + "\0", 'UTF-8'))
        s.send(bytes(user + "\0", 'UTF-8'))
        s.send(bytes(message + '\0', 'UTF-8'))

        # receive response with set timeout from server and close connection
        s.settimeout(TIMEOUT)
        try:
            response = int(s.recv(1).decode())
            if response == 0:
                message_id = int(s.recv(3).decode())
        except socket.timeout:
            # Handle a timeout exception
            sg.Popup(f'Timeout occured, no data received within {TIMEOUT} sec', title='TIMEOUT', button_type=5, auto_close=True, auto_close_duration=3)
            response = 2
        s.close()

        match response:
            case 0:
                print("SEND " + user + " " + message)
                window['_SERVER_'].print(f"s> SEND OK - MESSAGE {message_id}")
                return client.RC.OK
            case 1:
                window['_SERVER_'].print("s>  SEND FAIL / USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            # not only for case 2 but for any other
            case _:
                window['_SERVER_'].print("s> SEND FAIL")
                return client.RC.ERROR

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # * @param file    - file  to be sent

    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def sendAttach(user, message, file, window):
        window['_SERVER_'].print("s> SENDATTACH MESSAGE OK")
        print("SEND ATTACH " + user + " " + message + " " + file)
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def connectedUsers(window):
        window['_SERVER_'].print("s> CONNECTED USERS OK")
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def window_register():
        layout_register = [[sg.Text('Ful Name:'), sg.Input('Text', key='_REGISTERNAME_', do_not_clear=True, expand_x=True)],
                           [sg.Text('Alias:'), sg.Input('Text', key='_REGISTERALIAS_', do_not_clear=True, expand_x=True)],
                           [sg.Text('Date of birth:'),
                            sg.Input('', key='_REGISTERDATE_', do_not_clear=True, expand_x=True, disabled=True, use_readonly_for_disable=False),
                            sg.CalendarButton("Select Date", close_when_date_chosen=True, target="_REGISTERDATE_", format='%d-%m-%Y', size=(10, 1))],
                           [sg.Button('SUBMIT', button_color=('white', 'blue'))]
                           ]

        layout = [[sg.Column(layout_register, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window("REGISTER USER", layout, modal=True)

        while True:
            event, values = window.read()

            if (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                break

            if event == "SUBMIT":
                if (values['_REGISTERNAME_'] == 'Text'
                   or values['_REGISTERNAME_'] == ''
                   or len(values['_REGISTERNAME_']) > NAME_MAX_LENGTH - 1  # -2 beacuse of additional /0'
                   or values['_REGISTERALIAS_'] == 'Text'
                   or values['_REGISTERALIAS_'] == ''
                   or len(values['_REGISTERALIAS_']) > ALIAS_MAX_LENGTH - 1
                   or values['_REGISTERDATE_'] == ''
                   or datetime.strptime(values['_REGISTERDATE_'], "%d-%m-%Y") > datetime.now() - timedelta(days=1)):
                    sg.Popup('Registration error', title='Please fill in the fields to register.', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                client._username = values['_REGISTERNAME_']
                client._alias = values['_REGISTERALIAS_']
                client._date = values['_REGISTERDATE_']
                break
        window.Close()

    # *
    # * @brief Prints program usage

    @staticmethod
    def usage():
        print("Usage: python3 py -s <server> -p <port>")

    # *
    # * @brief Parses program execution arguments

    @staticmethod
    def parseArguments():
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', '--server-ip', type=str, help='Server IP', default=SERVER_IP)
        parser.add_argument('-p', '--server-port', type=int, help='Server Port', default=SERVER_PORT)
        args = parser.parse_args()

        if (args.server_ip is None):
            parser.error("Usage: python3 client.py -s <server> -p <port>")
            return False

        if ((args.server_port < 1024) or (args.server_port > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535")
            return False

        client._server = args.server_ip
        client._port = args.server_port

        return True

    def main():

        if (not client.parseArguments()):
            client.usage()
            exit()

        lay_col = [[sg.Button('REGISTER', expand_x=True, expand_y=True),
                    sg.Button('UNREGISTER', expand_x=True, expand_y=True),
                    sg.Button('CONNECT', expand_x=True, expand_y=True),
                    sg.Button('DISCONNECT', expand_x=True, expand_y=True),
                    sg.Button('CONNECTED USERS', expand_x=True, expand_y=True)],
                   [sg.Text('Dest:'), sg.Input('User', key='_INDEST_', do_not_clear=True, expand_x=True),
                    sg.Text('Message:'), sg.Input('Text', key='_IN_', do_not_clear=True, expand_x=True),
                    sg.Button('SEND', expand_x=True, expand_y=False)],
                   [sg.Text('Attached File:'), sg.In(key='_FILE_', do_not_clear=True, expand_x=True), sg.FileBrowse(),
                    sg.Button('SENDATTACH', expand_x=True, expand_y=False)],
                   [sg.Multiline(key='_CLIENT_', disabled=True, autoscroll=True, size=(60, 15), expand_x=True, expand_y=True),
                    sg.Multiline(key='_SERVER_', disabled=True, autoscroll=True, size=(60, 15), expand_x=True, expand_y=True)],
                   [sg.Button('QUIT', button_color=('white', 'red'))]]

        layout = [[sg.Column(lay_col, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window('Messenger', layout, resizable=True, finalize=True, size=(1000, 400))
        window.bind("<Escape>", "-ESCAPE-")

        while True:
            event, values = window.Read()

            if (event in (None, 'QUIT')) or (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                sg.Popup('Closing Client APP', title='Closing', button_type=5, auto_close=True, auto_close_duration=1)
                break

            # if (values['_IN_'] == '') and (event != 'REGISTER' and event != 'CONNECTED USERS'):
            #   window['_CLIENT_'].print("c> No text inserted")
            #   continue

            # if (client._alias is None or client._username is None or client._alias == 'Text'
            #     or client._username == 'Text' or client._date is None) and (event != 'REGISTER'):
            #     sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
            #     continue

            if (event == 'REGISTER'):
                client.window_register()
                if (client._alias is None or client._username is None or client._alias == 'Text' or client._username == 'Text' or client._date is None):
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=2)
                    continue

                window['_CLIENT_'].print('c> REGISTER ' + client._alias)
                client.register(client._alias, window)

            elif (event == 'UNREGISTER'):
                if (client._alias is None):
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                window['_CLIENT_'].print('c> UNREGISTER ' + client._alias)
                res = client.unregister(client._alias, window)
                print(res)
                if res == client.RC.OK:
                    client._username = None
                    client._alias = None
                    client._date = None

            elif (event == 'CONNECT'):
                if (client._alias is None):
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                window['_CLIENT_'].print('c> CONNECT ' + client._alias)
                client.connect(client._alias, window)

            elif (event == 'DISCONNECT'):
                if (client._alias is None):
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                window['_CLIENT_'].print('c> DISCONNECT ' + client._alias)
                client.disconnect(client._alias, window)

            elif (event == 'SEND'):

                if client._alias is None:
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue
                elif client._client_port is None:
                    sg.Popup('NOT CONNECTED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue
                else:
                    window['_CLIENT_'].print('c> SEND ' + values['_INDEST_'] + " " + values['_IN_'])

                if (values['_INDEST_'] == '' or values['_IN_'] == ''):
                    window['_CLIENT_'].print("Syntax error. Destination user and message cannot be empty.")
                elif (values['_INDEST_'] == 'User' or values['_IN_'] == 'Text'):
                    window['_CLIENT_'].print("Syntax error. Values should be other than default ones")
                elif (len(values['_IN_']) >= 255):
                    window['_CLIENT_'].print("Syntax error. Message should be shorter than 255 signs")
                else:
                    client.send(values['_INDEST_'], values['_IN_'], window)

            elif (event == 'SENDATTACH'):

                window['_CLIENT_'].print('c> SENDATTACH ' + values['_INDEST_'] + " " + values['_IN_'] + " " + values['_FILE_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_FILE_'] != ''):
                    client.sendAttach(values['_INDEST_'], values['_IN_'], values['_FILE_'], window)
                else:
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message> <attachedFile>")

            elif (event == 'CONNECTED USERS'):
                window['_CLIENT_'].print("c> CONNECTEDUSERS")
                client.connectedUsers(window)

            window.Refresh()

        window.Close()


if __name__ == '__main__':
    client.main()
    print("+++ FINISHED +++")
