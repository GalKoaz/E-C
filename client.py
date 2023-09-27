import socket
import os

host = '127.0.0.1'
port = 123144


def send_file(sock, filename):
    try:
        if os.path.isfile(filename):

            sock.send(filename.encode())

            with open(filename, "rb") as file:
                file_size = os.path.getsize(filename)
                file_size_bytes = file_size.to_bytes(8, byteorder="big")
                sock.send(file_size_bytes)

                while True:
                    chunk = file.read(1024)
                    if not chunk:
                        break
                    sock.send(chunk)
            print(f"File '{filename}' sent successfully.")
        else:
            print('File not found. Please enter a valid filename.')
        sock.send(b'EOF')
    except Exception as e:
        print(f"Error: {str(e)}")


def download_file(sock, filename):
    try:
        sock.send(filename.encode())

        with open(filename, "wb") as file:
            while True:
                chunk = sock.recv(1024)
                if not chunk:
                    break
                file.write(chunk)

        print(f"File '{filename}' received and saved successfully.")
    except Exception as e:
        print(f"Error: {str(e)}")


import socket


def receive_file_list(sock):
    try:

        file_list_length_str = sock.recv(16).decode()
        file_list_length = int(file_list_length_str)

        sock.send(b'ACK')

        # Receive the file list
        files_list = ""
        received_length = 0
        while received_length < file_list_length:
            chunk = sock.recv(1024).decode()
            if not chunk:
                break
            received_length += len(chunk)
            files_list += chunk
            print(f"Received chunk:\n{chunk}")

        # return files_list

    except socket.error as e:
        print(f"Socket error: {str(e)}")
    except Exception as e:
        print(f"Error: {str(e)}")



def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    while True:
        operation = input("Type 'Upload' to upload a file, 'Download' to download a file,'Files' to see the files "
                          "already uploaded, or 'exit' to exit: ")
        if operation == 'Upload':
            filename = input('Enter the filename you want to send: ')
            sock.send(b'upload')
            send_file(sock, filename)
        elif operation == 'Download':
            filename = input('Enter the filename you want to download: ')
            sock.send(b'download')
            download_file(sock, filename)
        elif operation == 'Files':
            sock.send(b'list_files')
            receive_file_list(sock)
        elif operation == 'exit':
            break
        else:
            print('Invalid operation. Please enter "Upload," "Download," or "exit".')

    sock.close()


if __name__ == '__main__':
    main()
