# reads a 4 color indexed PNG file,
# converts it to two 1-bit bitmaps 
# and servers it over http.
# The two bitmaps combined determine the pixel color:
# both 1 = black, both 0 = white, 1/0 = gray1, 0/1 = gray2
# The black bitmap is first in the response followed by the color bitmap withot any separator.
# Both bitmaps should be the same lenght

#pip install pypng

import png, array

BLACK=0
GRAY1=1
GRAY2=2
WHITE=3

from http.server import HTTPServer, BaseHTTPRequestHandler


class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        
        reader = png.Reader(filename='pik.png')
        w, h, pixels, metadata = reader.read_flat()

        black = bytearray()
        color = bytearray()

        blackByte=0
        colorByte=0
        bit=0

        for p in pixels:

            blackByte<<=1
            colorByte<<=1
            
            if p == BLACK:
                blackByte+=1
                colorByte+=1
            elif p == GRAY1:
                blackByte+=1
            elif p == GRAY2:
                colorByte+=1
            #elif p == WHITE:
                # both 0

            bit+=1
            if bit == 8:
                bit = 0
                black.append(blackByte)
                color.append(colorByte)
                blackByte=0
                colorByte=0

        self.wfile.write(black)
        self.wfile.write(color)


httpd = HTTPServer(('0.0.0.0', 1337), SimpleHTTPRequestHandler)
httpd.serve_forever()


