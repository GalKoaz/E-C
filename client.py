import socket
import os
import struct

host = '127.0.0.1'
port = 13019


def send_file(sock, filename):
    try:
        if os.path.isfile(filename):

            sock.send(filename.encode())

            with open(filename, "rb") as file:
                file_size = os.path.getsize(filename)
                file_size_bytes = struct.pack("!Q", file_size)
                sock.send(file_size_bytes)

                while file_size >= 0:
                    chunk = file.read(1024)
                    if not chunk:
                        break
                    sock.send(chunk)
                    file_size -= len(chunk)
            print(f"File '{filename}' sent successfully.")
        else:
            print('File not found. Please enter a valid filename.')

    except Exception as e:
        print(f"Error: {str(e)}")


def download_file(sock, filename):
    try:
        sock.send(filename.encode())

        file_size_str = sock.recv(1024)
        file_size = int(file_size_str)
        print(file_size)

        sock.send(b"ACK")

        with open(filename, "wb") as f:
            total_received = 0
            while total_received < file_size:
                data = sock.recv(1024)
                if not data:
                    break
                f.write(data)
                total_received += len(data)

        print(f"File '{filename}' received and saved successfully.")
    except ValueError:
        print(f"Error: Invalid file size received from the server.")
    except Exception as e:
        print(f"Error: {str(e)}")


def receive_file_list(sock):
    try:

        file_list_length_str = sock.recv(16).decode()
        file_list_length = int(file_list_length_str)

        sock.send(b'ACK')

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
            sock.send(b'upload')
            filename = input('Enter the filename you want to send: ')
            send_file(sock, filename)
        elif operation == 'Download':
            sock.send(b'download')
            filename = input('Enter the filename you want to download: ')
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