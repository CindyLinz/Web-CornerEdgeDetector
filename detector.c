#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void mono(uint8_t * buffer, int width, int height){
    uint8_t * buffer_end = buffer + width * height * 4;
    for(uint8_t * p = buffer; p<buffer_end; p+=4){
        uint8_t v = (uint8_t)(.299 * p[0] + .587 * p[1] + .114 * p[2] + .5);
        p[0] = p[1] = p[2] = v;
    }
}

void composite(uint8_t * a, uint8_t * b, int width, int height){
    int n = width * height;
    while(n--){
        if( b[3] )
            for(int i=0; i<4; ++i)
                a[i] = b[i];
        a += 4;
        b += 4;
    }
}

void detect(uint8_t * buffer, uint8_t * mark_corner, uint8_t * mark_edge, int width, int height, int window, float kappa, float flat){
    uint8_t * buffer_end = buffer + width * height * 4;

    float * mono = (typeof(mono)) malloc(sizeof(*mono) * width * height);
    typeof(mono) mono_end = mono + width * height;
    {
        typeof(mono) q = mono;
        for(typeof(buffer) p=buffer; p<buffer_end; p+=4, ++q)
            *q = (float)((.299 * p[0] + .587 * p[1] + .114 * p[2]) / 255.);
    }

    float * dx = (typeof(dx)) malloc(sizeof(*dx) * width * height);
    typeof(dx) dx_end = dx + width * height;
    float * dy = (typeof(dy)) malloc(sizeof(*dy) * width * height);
    typeof(dy) dy_end = dy + width + height;
    {
        typeof(dx) qx = dx;
        typeof(dy) qy = dy;
        for(typeof(mono) p=mono; p<mono_end; ++p, ++qx, ++qy){
            *qx = p[0] - p[-1];
            *qy = p[0] - p[-width];
        }
    }

    int s_len = (window*2+1)*(window*2+1);
    int s[s_len];
    for(int i=-window, k=0; i<=window; ++i)
        for(int j=-window; j<=window; ++j, ++k)
            s[k] = i*width+j;

    float * r = (typeof(r)) malloc(sizeof(*r) * width * height);
    typeof(r) r_end = r + width * height;
    {
        typeof(dx) px = dx;
        typeof(dy) py = dy;
        for(typeof(r) q=r; q<r_end; ++q, ++px, ++py){
            float m11 = 0, m12 = 0, m22 = 0;
            for(int d=0; d<s_len; ++d){
                m11 += px[s[d]] * px[s[d]];
                m12 += px[s[d]] * py[s[d]];
                m22 += py[s[d]] * py[s[d]];
            }
            *q = m11*m22 - m12*m12 - kappa * (m11+m22) * (m11+m22);
        }
    }

    {
        typeof(mark_corner) qc = mark_corner;
        typeof(mark_edge) qe = mark_edge;
        for(typeof(r) pr=r; pr<r_end; ++pr, qc+=4, qe+=4){
            if( *pr <= -flat ){
                // edge
                qc[0] = qc[1] = qc[2] = qc[3] = 0;

                qe[0] = qe[2] = 0;
                qe[1] = qe[3] = 255;
            }
            else if( *pr < flat ){
                // flat
                qc[0] = qc[1] = qc[2] = qc[3] = 0;
                qe[0] = qe[1] = qe[2] = qe[3] = 0;
            }
            else{
                // corner
                qe[0] = qe[1] = qe[2] = qe[3] = 0;
                int great = 1;
                for(int d=0; d<s_len; ++d)
                    if( s[d] )
                        if( pr[0]<pr[s[d]] ){
                            great = 0;
                            break;
                        }
                if( great )
                    qc[1] = 0;
                else
                    qc[1] = 255;
                qc[2] = 0;
                qc[0] = qc[3] = 255;
            }
        }

        for(int w=0; w<=window; ++w){
            qc = mark_corner;
            qe = mark_edge;
            for(int i=0; i<width; ++i, qc+=4, qe+=4)
                qc[width*w*4+3] = qe[width*w*4+3] = qc[width*(height-1-w)*4+3] = qe[width*(height-1-w)*4+3] = 0;
        }

        for(int w=0; w<=window; ++w){
            qc = mark_corner;
            qe = mark_edge;
            for(int i=0; i<height; ++i, qc+=width*4, qe+=width*4)
                qc[w*4+3] = qe[w*4+3] = qc[(width-1-w)*4+3] = qe[(width-1-w)*4+3] = 0;
        }
    }

    {
        typeof(buffer) q=buffer;
        for(typeof(mono) p=mono; p<mono_end; ++p, q+=4){
            q[0] = q[1] = q[2] = (typeof(q[0]))(*p * 255 + .5);
            q[3] = 255;
        }
    }

    free(r); free(dy); free(dx); free(mono);
}
