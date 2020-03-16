FROM dockcross/linux-x86:latest
COPY . /work
RUN chmod +x /work/docker/entrypoint.sh

ARG PLUGIN_VERSION

WORKDIR /work/build
RUN cmake .. -DPLUGIN_VERSION=${PLUGIN_VERSION}
ENTRYPOINT ["/work/docker/entrypoint.sh"]