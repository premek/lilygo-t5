FROM python:3-slim as builder
WORKDIR /app
RUN python3 -m venv /venv
ENV PATH="/venv/bin:$PATH"

RUN --mount=type=cache,target=/root/.cache/pip \
    --mount=type=bind,source=requirements.txt,target=requirements.txt \
    pip install -r requirements.txt


FROM python:3-slim
WORKDIR /app
COPY --from=builder /venv /venv
RUN apt-get update && apt-get install -y libcairo2
ENV PATH="/venv/bin:$PATH"
COPY src .

CMD ["python", "server.py"]
