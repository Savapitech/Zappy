FROM python:3.13.14-trixie
WORKDIR /app
COPY . .
ENTRYPOINT [ "python3.13", "zappy_ai.py" ]

# HOW TO USE #
# To build the image, run this :
# docker build --tag client .
# To run it, use it like so :
# docker run --net=host client:latest -h <host> -p <port> -n <team>
