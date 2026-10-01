Docker
------
docker build -t weather .
docker compose up --detach # run in background, port in compose.yaml

deploy without registry
---
docker image save -o image.tar weather
scp image.tar xxxxx:/opt/weather/
scp compose.yaml xxxxx:/opt/weather/
ssh xxxxx
cd /opt/weather
docker load < image.tar
rm image.tar
docker compose up --detach
