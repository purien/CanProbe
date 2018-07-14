# CanProbe
An open software for scanning the CAN Bus

The low cost probe (about 30$) comprises the following components
- An Arduino Mega2560
- An MCP2515 board (CAN probe)
- An OBDII plug

Communication via USB-Serial, 115200 bauds, 1 stop, no parity

Three operating modes (default = scan, set through the serial link)
- scan, dump CAN packets according to CanId filter (if any)
- diff dump differential CAN packets, according to CanId filter (if any)
- send, injection of CAN packets

Main commands (over serial link)
- empty line (CrLf), iddle mode
- scan CrLf, scan mode
- diff CrLf, differential scan mode
- send CanId Len Data Mask UseCRC,  injection mode 
- filter CanId1...CanIdn CrLf, set a list of CANId filters
- mask  mask1...maskn CrLf, set a list of filter masks
- can CanId Len Data CrLf, send a CAN packet
- iso CanIdReq CanIdResp Len Data CrLf, send an ISO-TP packet (in Iddle mode only)
