#include "CStreamer.h"

#include <stdio.h>

CStreamer::CStreamer(SOCKET aClient, u_short width, u_short height) : m_Client(aClient)
{
    printf("Creating TSP streamer\n");
    m_RtpServerPort  = 0;
    m_RtcpServerPort = 0;
    m_RtpClientPort  = 0;
    m_RtcpClientPort = 0;

    m_SequenceNumber = 0;
    m_Timestamp      = 0;
    m_SendIdx        = 0;
    m_TCPTransport   = false;

    m_RtpSocket = NULLSOCKET;
    m_RtcpSocket = NULLSOCKET;

    m_width = width;
    m_height = height;
    m_jpegType = 0; // 0 for 4:2:2, 1 for 4:2:0
    m_prevMsec = 0;
};

CStreamer::~CStreamer()
{
    udpsocketclose(m_RtpSocket);
    udpsocketclose(m_RtcpSocket);
};

int CStreamer::SendRtpPacket(unsigned const char * jpeg, int jpegLen, int fragmentOffset, BufPtr quant0tbl, BufPtr quant1tbl)
{
#define KRtpHeaderSize 12           // size of the RTP header
#define KJpegHeaderSize 8           // size of the special JPEG payload header

#define MAX_FRAGMENT_SIZE 1100 // Pick fragment size carefully for MTU safety
    int fragmentLen = MAX_FRAGMENT_SIZE;
    if(fragmentLen + fragmentOffset > jpegLen) // Shrink last fragment if needed
        fragmentLen = jpegLen - fragmentOffset;

    bool isLastFragment = (fragmentOffset + fragmentLen) == jpegLen;

    // Do we have custom quant tables? If so include them per RFC 2435
    bool includeQuantTbl = quant0tbl && quant1tbl && fragmentOffset == 0;
    uint8_t q = includeQuantTbl ? 128 : 0x5e;

    static char RtpBuf[2048]; // Single threaded FreeRTOS worker task
    int RtpPacketSize = fragmentLen + KRtpHeaderSize + KJpegHeaderSize + (includeQuantTbl ? (4 + 64 * 2) : 0);

    memset(RtpBuf, 0x00, sizeof(RtpBuf));
    // Prepare the first 4 byte of the packet (RTP over RTSP interleaved mode)
    RtpBuf[0]  = '$';        // magic number
    RtpBuf[1]  = 0;          // multiplexed subchannel
    RtpBuf[2]  = (RtpPacketSize & 0x0000FF00) >> 8;
    RtpBuf[3]  = (RtpPacketSize & 0x000000FF);

    // Prepare the 12 byte RTP header
    RtpBuf[4]  = 0x80;                               // RTP version
    RtpBuf[5]  = 0x1a | (isLastFragment ? 0x80 : 0x00); // JPEG payload (26) and marker bit
    RtpBuf[6]  = m_SequenceNumber >> 8;
    RtpBuf[7]  = m_SequenceNumber & 0x0FF;
    RtpBuf[8]  = (m_Timestamp & 0xFF000000) >> 24;
    RtpBuf[9]  = (m_Timestamp & 0x00FF0000) >> 16;
    RtpBuf[10] = (m_Timestamp & 0x0000FF00) >> 8;
    RtpBuf[11] = (m_Timestamp & 0x000000FF);
    RtpBuf[12] = 0x13;                               // 4 byte SSRC
    RtpBuf[13] = 0xf9;
    RtpBuf[14] = 0x7e;
    RtpBuf[15] = 0x67;

    // Prepare the 8 byte payload JPEG header (RFC 2435)
    RtpBuf[16] = 0x00;                               // type specific
    RtpBuf[17] = (fragmentOffset & 0x00FF0000) >> 16; // 3 byte fragmentation offset
    RtpBuf[18] = (fragmentOffset & 0x0000FF00) >> 8;
    RtpBuf[19] = (fragmentOffset & 0x000000FF);

    /* Sampling factor: Type 0 = 4:2:2, Type 1 = 4:2:0 */
    RtpBuf[20] = m_jpegType;
    RtpBuf[21] = q;                                  // quality scale factor (128 with quant table)
    RtpBuf[22] = (uint8_t)(m_width / 8);             // width  / 8
    RtpBuf[23] = (uint8_t)(m_height / 8);            // height / 8

    int headerLen = 24; // Including jpeg header but not quant table header
    if (includeQuantTbl) {
        RtpBuf[24] = 0; // MBZ
        RtpBuf[25] = 0; // 8 bit precision
        RtpBuf[26] = 0; // MSB of length

        int numQantBytes = 64; // Two 64 byte tables
        RtpBuf[27] = 2 * numQantBytes; // LSB of length

        headerLen += 4;

        memcpy(RtpBuf + headerLen, quant0tbl, numQantBytes);
        headerLen += numQantBytes;

        memcpy(RtpBuf + headerLen, quant1tbl, numQantBytes);
        headerLen += numQantBytes;
    }

    // Append JPEG scan data to RTP buffer
    memcpy(RtpBuf + headerLen, jpeg + fragmentOffset, fragmentLen);
    fragmentOffset += fragmentLen;

    m_SequenceNumber++;

    IPADDRESS otherip;
    IPPORT otherport;
    socketpeeraddr(m_Client, &otherip, &otherport);

    // RTP marker bit must be set on last fragment
    if (m_TCPTransport)
        socketsend(m_Client, RtpBuf, RtpPacketSize + 4);
    else
        udpsocketsend(m_RtpSocket, &RtpBuf[4], RtpPacketSize, otherip, m_RtpClientPort);

    return isLastFragment ? 0 : fragmentOffset;
};

void CStreamer::InitTransport(u_short aRtpPort, u_short aRtcpPort, bool TCP)
{
    m_RtpClientPort  = aRtpPort;
    m_RtcpClientPort = aRtcpPort;
    m_TCPTransport   = TCP;

    if (!m_TCPTransport)
    {
        for (u_short P = 6970; P < 0xFFFE; P += 2)
        {
            m_RtpSocket = udpsocketcreate(P);
            if (m_RtpSocket)
            {
                m_RtcpSocket = udpsocketcreate(P + 1);
                if (m_RtcpSocket)
                {
                    m_RtpServerPort  = P;
                    m_RtcpServerPort = P+1;
                    break;
                }
                else
                {
                    udpsocketclose(m_RtpSocket);
                    udpsocketclose(m_RtcpSocket);
                };
            }
        };
    };
};

u_short CStreamer::GetRtpServerPort()
{
    return m_RtpServerPort;
};

u_short CStreamer::GetRtcpServerPort()
{
    return m_RtcpServerPort;
};

void CStreamer::streamFrame(unsigned const char *data, uint32_t dataLen, uint32_t curMsec)
{
    if (m_prevMsec == 0)
        m_prevMsec = curMsec;

    uint32_t deltams = (curMsec >= m_prevMsec) ? curMsec - m_prevMsec : 100;
    m_prevMsec = curMsec;

    BufPtr qtable0 = nullptr;
    BufPtr qtable1 = nullptr;
    uint8_t detectedType = m_jpegType;
    uint16_t detectedW = m_width;
    uint16_t detectedH = m_height;

    if (!decodeJPEGfile(&data, &dataLen, &qtable0, &qtable1, &detectedType, &detectedW, &detectedH)) {
        return;
    }

    if (detectedW > 0 && detectedH > 0) {
        m_width = detectedW;
        m_height = detectedH;
    }
    m_jpegType = detectedType;

    int offset = 0;
    do {
        offset = SendRtpPacket(data, dataLen, offset, qtable0, qtable1);
    } while (offset != 0);

    uint32_t units = 90000; // 90kHz clock per RFC 2435
    m_Timestamp += (units * deltams / 1000);

    m_SendIdx++;
    if (m_SendIdx > 1) m_SendIdx = 0;
};

bool decodeJPEGfile(BufPtr *start, uint32_t *len, BufPtr *qtable0, BufPtr *qtable1, uint8_t *type, uint16_t *width, uint16_t *height) {
    if (!start || !*start || !len || *len < 4) return false;

    BufPtr bytes = *start;
    uint32_t totalLen = *len;
    *qtable0 = nullptr;
    *qtable1 = nullptr;

    if (bytes[0] != 0xFF || bytes[1] != 0xD8) {
        return false;
    }

    uint32_t pos = 2;
    BufPtr sos_start = nullptr;
    uint32_t sos_len = 0;

    while (pos + 4 <= totalLen) {
        if (bytes[pos] != 0xFF) {
            pos++;
            continue;
        }
        uint8_t marker = bytes[pos + 1];
        pos += 2;

        if (marker == 0xD8 || marker == 0xD9 || marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7)) {
            continue;
        }

        if (pos + 2 > totalLen) break;
        uint16_t blockLen = (bytes[pos] << 8) | bytes[pos + 1];
        if (blockLen < 2 || pos + blockLen > totalLen) {
            break;
        }

        if (marker == 0xDB) { // DQT
            uint32_t dpos = pos + 2;
            uint32_t dend = pos + blockLen;
            while (dpos + 65 <= dend) {
                uint8_t qinfo = bytes[dpos++];
                uint8_t qid = qinfo & 0x0F;
                if (qid == 0 && *qtable0 == nullptr) {
                    *qtable0 = &bytes[dpos];
                } else if (qid == 1 && *qtable1 == nullptr) {
                    *qtable1 = &bytes[dpos];
                }
                dpos += 64;
            }
        } else if (marker == 0xC0) { // SOF0
            if (blockLen >= 8) {
                uint16_t h = (bytes[pos + 3] << 8) | bytes[pos + 4];
                uint16_t w = (bytes[pos + 5] << 8) | bytes[pos + 6];
                if (w > 0 && h > 0) {
                    if (width) *width = w;
                    if (height) *height = h;
                }
                if (blockLen >= 10 && type) {
                    uint8_t samp = bytes[pos + 9]; // Y sampling factor
                    if (samp == 0x22) {
                        *type = 1; // 4:2:0
                    } else {
                        *type = 0; // 4:2:2
                    }
                }
            }
        } else if (marker == 0xDA) { // SOS
            sos_start = &bytes[pos + blockLen];
            sos_len = totalLen - (pos + blockLen);
            break;
        }

        pos += blockLen;
    }

    if (!sos_start || sos_len == 0) {
        return false;
    }

    BufPtr scan_ptr = sos_start;
    uint32_t scan_rem = sos_len;
    bool found_eoi = false;
    uint32_t payload_len = 0;

    for (uint32_t i = 0; i + 1 < scan_rem; i++) {
        if (scan_ptr[i] == 0xFF) {
            uint8_t next = scan_ptr[i + 1];
            if (next == 0xD9) { // EOI
                found_eoi = true;
                payload_len = i;
                break;
            }
            if (next != 0x00 && !(next >= 0xD0 && next <= 0xD7)) {
                payload_len = i;
                found_eoi = true;
                break;
            }
        }
    }

    if (!found_eoi) {
        payload_len = scan_rem;
    }

    *start = sos_start;
    *len = payload_len;
    return true;
}


