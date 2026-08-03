#include "OV2640Streamer.h"
#include <assert.h>

OV2640Streamer::OV2640Streamer(SOCKET aClient, OV2640 &cam) : CStreamer(aClient, cam.getWidth(), cam.getHeight()), m_cam(cam)
{
    printf("Created streamer width=%d, height=%d\n", cam.getWidth(), cam.getHeight());
}

void OV2640Streamer::streamImage(uint32_t curMsec)
{
    m_cam.run();

    int w = m_cam.getWidth();
    int h = m_cam.getHeight();
    if (w > 0 && h > 0) {
        m_width = w;
        m_height = h;
    }

    if (m_cam.lockFrame()) {
        BufPtr bytes = m_cam.getfb();
        size_t size = m_cam.getSize();

        if (bytes != nullptr && size > 0) {
            streamFrame(bytes, size, curMsec);
        }
        m_cam.unlockFrame();
    }
}
