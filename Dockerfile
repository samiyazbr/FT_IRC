
FROM alpine:3.18

WORKDIR /app

RUN apk update && \
    apk add --no-cache g++ make cmake && \
    rm -rf /var/cache/apk/*

COPY . /app


RUN make

EXPOSE 3001

CMD ["./ircserv", "3001", "12345"]
