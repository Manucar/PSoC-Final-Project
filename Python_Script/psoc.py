import sys
import glob
import serial
from tabulate import tabulate
import numpy as np
import struct
import matplotlib
import matplotlib
matplotlib.use('Qt4Agg')
from matplotlib import pyplot as plt


BAUDRATE = 115200

""" 'R' = reset EEPROM
    'C' = request control register status of the EEPROM
    'L' + 'logID'= request specific log by ID
    'N' = request number of logs stored in the EEPROM
"""
COMMAND_LIST = ['R', 'C', 'L', 'N']


class UART(serial.Serial):
    def __init__(self, baudrate):
        # Initialize python serial class
        super().__init__(port=self.scan_ports()[0], baudrate=baudrate, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS, timeout=0)

    def scan_ports(self):

        # Check operating system
        if sys.platform.startswith('win'):
            ports = ['COM%s' % (i + 1) for i in range(256)]
        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            # this excludes your current terminal "/dev/tty"
            ports = glob.glob('/dev/tty[A-Za-z]*')
        elif sys.platform.startswith('darwin'):
            ports = glob.glob('/dev/tty.*')
        else:
            raise EnvironmentError('Unsupported platform, please insert your port name manually.')

        # Find COM port
        result = []
        for port in ports:
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, serial.SerialException):
                pass

        # Return list of ports
        return result


class LogMessage:
    def __init__(self, data_stream):
        self.msg_size = 64
        self.parse_message(data_stream)

    def parse_message(self, stream):
        self.id = stream[0]
        self.int_reg = hex(stream[1])
        self.parse_timestamp(stream)
        self.parse_payload(stream)

    def parse_timestamp(self, stream):
        low_reg = stream[2]
        high_reg = stream[3]
        self.timestamp = low_reg | (high_reg << 8)

    def parse_payload(self, stream):
        self.x = []
        self.y = []
        self.z = []

        for i in range(5):
            # Get current page data payload
            page_data = stream[4 + i * 64: (i + 1) * 64]
            # Append xyz data values
            self.x += [x for x in page_data[0:self.msg_size:3]]
            self.y += [y for y in page_data[1:self.msg_size:3]]
            self.z += [z for z in page_data[2:self.msg_size:3]]

        # Remove zero padding at the end of each data axis
        self.x = self.x[:-4]
        self.y = self.y[:-4]
        self.z = self.z[:-4]

    def print_log(self):
        # Print log message header information
        print(tabulate([[self.id, self.timestamp, self.int_reg]], ["LOG_ID", "Timestamp (s)", "INT1_REG"], tablefmt="grid"))

        # Setting the x coordinate as timestamp of the data, each sample every 10 ms --> 0.96s total
        x_coord = np.arange(0, 0.96, 0.01)

        # potting the points
        plt.plot(x_coord, self.x)
        plt.plot(x_coord, self.y)
        plt.plot(x_coord, self.z)

        plt.ylabel('LSB [16 mg]')
        plt.xlabel('Time [s]')
        plt.legend(['x', 'y', 'z'], loc='upper left')

        plt.show()


class PsocController:
    def __init__(self):
        self.log_number = 0

    def print_menu(self):
        print("#" * 70)
        print("\nChoose a command from the list:\n")
        print("\tR = reset EEPROM \n\tC = request control register status of the EEPROM\n\tL = request specific log by ID\n\tN = request number of logs stored in the EEPROM\n")

    def print_ctrl_reg(self, reg):
        # Convert the ctr_reg in fixed length binary representation
        bits = "{0:{fill}4b}".format(reg, fill='0')
        # Print a table representing each bit of the register
        print(tabulate([[bits[0], bits[1], bits[2], bits[3], ]], ["Reset flag", "Send flag", "Config Mode", "Start/Stop mode"], tablefmt="grid"))

    def command_handling(self, command):
        # Check if valid command
        if (command in COMMAND_LIST):

            if(command == 'C' or command == 'N'):
                # Write PSoC read command
                uart_module.write(command.encode())
                # Read PSoC response
                res = uart_module.read()
                while not (res):
                    res = uart_module.read()

                if(command == 'C'):
                    # print(res[0])
                    self.print_ctrl_reg(res[0])
                else:
                    self.log_number = struct.unpack('<1B', res)[0]
                    print("Number of LOG stored in the EEPROM: " + str(self.log_number) + "\n")

            elif(command == 'L'):
                # Read requested log by ID
                print("Enter Log ID number[0 index]: ")
                log_id = int(input('> '))
                # Check if there are log stored in the EEPROM
                if(self.log_number != 0):
                    if(log_id <= self.log_number - 1):
                        # Send read log command and id to PSoC
                        uart_module.write(command.encode())
                        uart_module.write(struct.pack('B', log_id))

                        # Read all log messages and store data in buffer
                        buffer = []
                        # Number of bytes to read via UART --> 320 bytes
                        read_counter = 64 * 5
                        header_list = [0, 63, 62, 61]
                        while(read_counter > 0):
                            # Check only valid bytes from UART (avoid reading null bytes)
                            try:
                                # Read byte
                                raw_byte = uart_module.read()

                                # Check if read byte is in header
                                if((read_counter % 64) in header_list):
                                    # Unpack as unsigned integer of 8 bit
                                    buffer.append(struct.unpack('<1B', raw_byte)[0])
                                # Else read byte is data --> has sign
                                else:
                                    # Unpack as signed integer of 8 bit
                                    buffer.append(struct.unpack('<1b', raw_byte)[0])

                                # Decrease counter
                                read_counter -= 1

                            except Exception:
                                # Do nothing
                                pass

                        # Create log message class instance
                        log = LogMessage(buffer)
                        log.print_log()
                    else:
                        print("Wrong Log ID selected, recheck with 'N' command.\n")
                else:
                    print("No Log actually stored in the EEPROM, recheck with 'N' command.\n")

            elif(command == 'R'):

                # Send reset command to PSoC
                uart_module.write(command.encode())

                # Read PSoC response
                ack = uart_module.read()
                while not (ack):
                    ack = uart_module.read()

                if ack.decode() == 'K':
                    print("EEPROM log memory has been erased.")
                else:
                    print("EEPROM log memory reset failed.")

        else:
            print("Invalid command.\n")


if __name__ == "__main__":

    # Initialize UART connection
    uart_module = UART(baudrate=BAUDRATE)
    psoc_module = PsocController()

    # Loop until connection is active
    while(uart_module.isOpen()):
        psoc_module.print_menu()
        command = input('> ')

        psoc_module.command_handling(command)
