FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
    samba \
    smbclient \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /libraryit/{ebooks,papers,sourcecode,docs}

RUN groupadd readonly

RUN useradd -m member && \
    echo "member:member123" | chpasswd && \
    usermod -aG readonly member

RUN useradd -m contributor && \
    echo "contributor:contrib456" | chpasswd && \
    usermod -aG staff contributor

RUN useradd -m librarian && \
    echo "librarian:lib789" | chpasswd && \
    usermod -aG staff librarian

RUN (echo member123; echo member123) | smbpasswd -a -s member
RUN (echo contrib456; echo contrib456) | smbpasswd -a -s contributor
RUN (echo lib789; echo lib789) | smbpasswd -a -s librarian

COPY smb.conf /etc/samba/smb.conf
COPY entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh

EXPOSE 445

CMD ["/entrypoint.sh"]
