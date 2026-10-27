Server
------
- generates a SVG file using a SVG template and data downloaded from a weather forecast API
- converts the SVG to a bitmap using cairosvg
- converts the bitmap to a format understood by the display
- serves the bitmap on a HTTP server

Development
-----------
- install pip and cairo `apt install python3-pip cairosvg`
- run `./server/setup.sh` to install python dependencies
- `nohup ./server/server.py`

Deployment
----------
  docker build -t weather .
  docker compose up --detach # run in background, port in compose.yaml

deploy without registry:

  docker image save -o image.tar weather
  scp image.tar xxxxx:/opt/weather/
  scp compose.yaml xxxxx:/opt/weather/
  ssh xxxxx
  cd /opt/weather
  docker load < image.tar
  rm image.tar
  docker compose up --detach
