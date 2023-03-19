from socket import *
import struct
from sys import *

MAX_BUF_LEN = 1024
MIN_MC_PORT = 1024
MAX_MC_PORT = 65535

if len(argv) == 1 or \
    (argv[1] == 'transmit' and len(argv) != 6) or \
    (argv[1] == 'receive' and len(argv) != 5):
    print("Multicast Testing Tool Version 2.0\n")
    print("This tool is derived from multiple sources on network socket programming and has been enhanced to")
    print("support the flow of multicast packets across multiple Windows and Linux-based operating systems.\n")
    print("Usage: mctest [OPTIONS] [PARAMETERS]\n")
    print("where  OPTIONS    := { transmit | receive }\n"
            "       PARAMETERS := transmit { mcast_ip | local_if_ip | mcast_port [1024-65535] | mcast_ttl [0-255] }\n"
            "                     receive  { mcast_ip | local_if_ip | mcast_port [1024-65535] }\n")
    print("Usage example: To test the flow of channel 239.23.5.3 over UDP 48888 across multiple router boundaries\n"
          "               On the transmitting node: mctest transmit 239.23.5.3 10.23.238.10 48888 16\n"
          "               On the receiving node:    mctest receive 239.23.5.3 10.23.238.10 48888\n")
    exit(1)

# BEG: "transmit" option
elif argv[1] == 'transmit' and len(argv) == 6:
    mc_ip_str = argv[2]
    if_ip_str = argv[3]
    mc_port = int(argv[4])
    mc_ttl = int(argv[5])
    if_ip = inet_aton(if_ip_str)

    if not MIN_MC_PORT <= mc_port <= MAX_MC_PORT:
        print("Invalid port number argument,", mc_port, ".")
        print("Valid range is between", MIN_MC_PORT, "and", MAX_MC_PORT, ".")
        exit(1)

    # create a socket for sending to the multicast address
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
    print("\n\nCreate UDP multicast socket...OK")

    # set local interface IP address
    sock.setsockopt(IPPROTO_IP, IP_MULTICAST_IF, if_ip)
    print("Transmit over IP", if_ip_str + "...OK")

    # set TTL (time to live)
    sock.setsockopt(IPPROTO_IP, IP_MULTICAST_TTL, mc_ttl)
    print("Set TTL to", str(mc_ttl) + "...OK")

    # build the multicast group address
    mc_group = (mc_ip_str, mc_port)
    print("Set multicast group to:", str(mc_group) + "...OK\n")

    tx_buf = ''
    while tx_buf != "eos\n":
        tx_buf = input('Enter message: ') + '\n'
        tx_buf_len = len(tx_buf)

        # BEG: send string to multicast address */
        if tx_buf != "eos\n":
            sock.sendto(tx_buf.encode(), mc_group)
            print("Sent", tx_buf_len, "bytes from", mc_ip_str + ".\n");
        # END: send string to multicast address */
    sock.close()
# END: "transmit" option

# BEG: "receive" option
elif argv[1] != "receive" or len(argv) == 5:

    mc_ip_str = argv[2]
    if_ip_str = argv[3]
    mc_port = int(argv[4])

    # Validate the port range
    if not MIN_MC_PORT <= mc_port <= MAX_MC_PORT:
        print("Invalid port number argument,", mc_port, ".")
        print("Valid range is between", MIN_MC_PORT, "and", MAX_MC_PORT, + ".")
        exit(1)

    # Create UDP multicast socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)

    # Bind multicast address to socket
    mc_group = (mc_ip_str, mc_port)
    sock.bind(mc_group)

    # Set local interface IP address
    if_ip = inet_aton(if_ip_str)
    sock.setsockopt(IPPROTO_IP, IP_MULTICAST_IF, if_ip)

    # Construct IGMP join request structure
    mc_req = struct.pack("4s4s", inet_aton(mc_ip_str), inet_aton(if_ip_str))

    # Send an ADD MEMBERSHIP message via setsockopt()
    sock.setsockopt(IPPROTO_IP, IP_ADD_MEMBERSHIP, mc_req)

    print("\n\nWaiting to receive a stream of multicast packets over interface IP " + if_ip_str, "(CTRL-C to quit):\n")

    while True:
        # Wait to receive a packet
        rx_buf, tx_ip = sock.recvfrom(MAX_BUF_LEN)

        # Output received string
        tx_ip_str = tx_ip[0]
        print("Received", len(rx_buf), "bytes in group", mc_ip_str + " from speaker", tx_ip_str + ":", rx_buf.decode(), end='')

    # Send a DROP MEMBERSHIP message via setsockopt()
    sock.setsockopt(IPPROTO_IP, IP_DROP_MEMBERSHIP, mc_req)
    sock.close()
 #END: "receive" option
