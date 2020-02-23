FROM dockcross/linux-x86:latest
COPY . /work
RUN chmod +x /work/docker/entrypoint.sh

WORKDIR /work/build
RUN cmake ..
ENTRYPOINT ["/work/docker/entrypoint.sh"]