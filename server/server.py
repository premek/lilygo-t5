#!/usr/bin/env python3
import unittest
import base64
import io
import os
import sys
from urllib.parse import urlparse, parse_qsl
from http.server import HTTPServer, BaseHTTPRequestHandler
import requests

from cairosvg import svg2png
from PIL import Image

filedir = os.path.dirname(os.path.realpath(__file__))


class InputValidationException(Exception):
    pass


class FetchException(Exception):
    pass


def fetch_yr(lat, lon, alt):
    url = f"https://api.met.no/weatherapi/locationforecast/2.0/complete?lat={lat}&lon={lon}&altitude={alt}"
    headers = {
        "User-Agent": "meteo/0.1 github.com/premek",
    }
    response = requests.get(url, headers=headers, timeout=3)
    if response.status_code != 200:
        raise FetchException("met.no response: " + str(response.status_code) + ": " + str(response.content))
    return response.json()


def file_to_bytes(name):
    with open(name, "rb") as file:
        return file.read()


def file_to_str(name):
    with open(name, "r", encoding="utf8") as file:
        return file.read()


def ico(name):
    bytez = file_to_bytes(f"{filedir}/yr/ico/png/{name}.png")
    return "data:image/png;base64," + base64.b64encode(bytez).decode()


def minmax(min_value, max_value):
    rmin = round(min_value)
    rmax = round(max_value)
    if rmin == rmax:
        return f"{rmin}"
    return f"{rmin}-{rmax}"


def ms2kmh(meters_per_second):
    # m/s -> km/h
    return meters_per_second * 3.6


def feelslike(temp, wind_ms):
    # Wind chill factor is only used when the temperature is < 10°C and the wind speed is > 1.33 m/s
    if temp < 10 and wind_ms > 1.33:
        wind_pow = pow(ms2kmh(wind_ms), 0.16)
        return round(13.12 + 0.6215 * temp - 11.37 * wind_pow + 0.3965 * temp * wind_pow)
    # Heat index is only used when the temperature is > 26°C and humidity > 40 %,
    return round(temp)


def get_variables(data):
    current = data["properties"]["timeseries"][0]["data"]
    return {
        "#temp#": str(round(current["instant"]["details"]["air_temperature"])),
        "#wind#": str(round(current["instant"]["details"]["wind_speed"])),
        "#feel#": str(
            feelslike(current["instant"]["details"]["air_temperature"], current["instant"]["details"]["wind_speed"])
        ),
        "#ico#": ico(current["next_1_hours"]["summary"]["symbol_code"]),
        "#ico2#": ico(current["next_6_hours"]["summary"]["symbol_code"]),
        # "#ico3#": ico(current["next_12_hours"]["summary"]["symbol_code"]),
        # TEMP=$(round "$(jq '.properties.timeseries[0].data.instant.details.air_temperature' "$T")")
        # WIND=$(jq '.properties.timeseries[0].data.instant.details.wind_speed' "$T")
        "#r1#": str(round(current["next_1_hours"]["details"]["precipitation_amount"], 1)),
        "#r6#": str(round(current["next_6_hours"]["details"]["precipitation_amount"], 1)),
        "#t6#": minmax(
            current["next_6_hours"]["details"]["air_temperature_min"],
            current["next_6_hours"]["details"]["air_temperature_max"],
        ),
        "#time#": data["properties"]["timeseries"][0]["time"],
    }.items()


def get_svg(data):
    res = file_to_str(f"{filedir}/screen_template.svg")
    for variable, value in get_variables(data):
        res = res.replace(variable, value)
    return res


def to_png(svg: str):
    return svg2png(
        bytestring=svg, scale=1, unsafe=1
    )  # unsafe to allow loading images. we must control the svg template to be safe


# reads a PNG file and converts it to two 1-bit bitmaps
# The two bitmaps combined determine the pixel color:
# both 1 = black, both 0 = white, 1/0 = gray1, 0/1 = gray2
# The black bitmap is first in the response followed by the color bitmap without any separator.
# Both bitmaps should be the same lenght


def to_eink(png: bytes):
    img = Image.open(io.BytesIO(png)).convert(colors=4)
    pixels = list(map(lambda x: x[3], list(img.getdata())))
    black = bytearray()
    color = bytearray()

    black_byte = 0
    color_byte = 0
    bit = 0

    for pixel in pixels:
        black_byte <<= 1
        color_byte <<= 1

        if pixel > 125:
            # black
            black_byte += 1
            color_byte += 1
        elif pixel > 80:
            # gray1
            black_byte += 1
        elif pixel > 35:
            # gray2
            color_byte += 1
        else:
            # white
            pass

        bit += 1
        if bit == 8:
            bit = 0
            black.append(black_byte)
            color.append(color_byte)
            black_byte = 0
            color_byte = 0

    return black + color


class Handler(BaseHTTPRequestHandler):
    def send(self, content_type, resp):
        self.send_response(200)
        self.send_header("Content-type", content_type)
        self.end_headers()
        self.wfile.write(resp)

    def send_svg(self, data):
        self.send("image/svg+xml", get_svg(data).encode())

    def send_png(self, data):
        self.send("image/png", to_png(get_svg(data)))

    def send_eink(self, data):
        self.send("application/octet-stream", to_eink(to_png(get_svg(data))))

    def get_handler(self, fmt):
        return {
            "eink": self.send_eink,
            "svg": self.send_svg,
            "png": self.send_png,
        }[fmt]

    def parse_params(self, query):
        try:
            query = dict(parse_qsl(query))
            lat = float(query.get("lat"))
            lon = float(query.get("lon"))
            alt = int(query.get("alt"))
            handler = self.get_handler(query.get("format"))
            return lat, lon, alt, handler
        except Exception as ex:
            raise InputValidationException(ex) from ex

    def weather(self, query):
        lat, lon, alt, handler = self.parse_params(query)
        data = fetch_yr(lat, lon, alt)
        handler(data)

    def do_GET(self):  # pylint: disable=invalid-name
        try:
            url = urlparse(self.path)
            if url.path == "/weather":
                self.weather(url.query)
            else:
                self.send_response(404)
                self.end_headers()

        except InputValidationException as ex:
            exception(self, ex, 400)
        except Exception as ex:  # pylint: disable=broad-exception-caught
            exception(self, ex, 500)


def exception(self, ex, resp):
    print(f"{type(ex)}: {ex}", file=sys.stderr)
    self.send_response(resp)
    self.end_headers()


def main():
    port = 1337
    request_handler = Handler
    request_handler.server_version = ""
    request_handler.sys_version = ""

    httpd = HTTPServer(("0.0.0.0", port), request_handler)
    print(f"http://localhost:{port}/weather?format=png&lat=25.276987&lon=55.296249&alt=2")
    httpd.serve_forever()


if __name__ == "__main__":
    main()


# tests
class MyTest(unittest.TestCase):
    def test_speed(self):
        self.assertEqual(ms2kmh(1), 3.6)

    def test_feelslike(self):
        self.assertEqual(feelslike(1, 9), -5)
        self.assertEqual(feelslike(1, 1.3), 1)
        self.assertEqual(feelslike(10, 9), 10)
        self.assertEqual(feelslike(3, 3), 0)
        self.assertEqual(feelslike(9, 2), 8)
        self.assertEqual(feelslike(4, 5), 0)
        self.assertEqual(feelslike(0, 5), -5)
        self.assertEqual(feelslike(-4, 4), -9)
        self.assertEqual(feelslike(-2, 4), -7)
        self.assertEqual(feelslike(21.8, 9), 22)  # round even if returning the input value

    def test_minmax(self):
        self.assertEqual(minmax(1, 2), "1-2")
        self.assertEqual(minmax(2, 1), "2-1")
        self.assertEqual(minmax(1, 1), "1")
        self.assertEqual(minmax(1.9, 2.1), "2")
        self.assertEqual(minmax(1.9, 10.1), "2-10")
        self.assertEqual(minmax(-1.9, 10.1), "-2-10")
        self.assertEqual(minmax(1.9, -10.1), "2--10")
        self.assertEqual(minmax(-1.9, -10.1), "-2--10")
