# replace 'templatebot' with the name of your bot in the paths

FROM brainboxdotcc/dpp:latest

WORKDIR /usr/src/guidingLight

COPY . .

WORKDIR /usr/src/guidingLight/build

RUN cmake ..
RUN make -j$(nproc)

ENTRYPOINT [ "/usr/src/guidingLight/build/guidingLight" ]
