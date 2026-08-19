#include "OV2640Streamer.h"
#include <assert.h>

OV2640Streamer::OV2640Streamer(SOCKET aClient, OV2640 &cam) : CStreamer(aClient, cam.getWidth(), cam.getHeight()), m_cam(cam)
{
    printf("Created streamer width=%d, height=%d\n", cam.getWidth(), cam.getHeight());
}

void OV2640Streamer::streamImage(uint32_t curMsec)
{
    camera_fb_t *fb = m_cam.getFrame();
    if (fb != nullptr) {
        if (fb->buf != nullptr && fb->len > 0) {
            if (fb->width > 0 && fb->height > 0) {
                m_width = fb->width;
                m_height = fb->height;
            }
            streamFrame(fb->buf, fb->len, curMsec);
        }
        m_cam.returnFrame(fb);
    }
}

