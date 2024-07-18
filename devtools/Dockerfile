ARG IMAGE_NAME

FROM $IMAGE_NAME as base

ARG DEBIAN_FRONTEND=noninteractive

ENV LANG=C.UTF-8

RUN apt-get update
RUN apt-get install -y unzip
RUN apt-get install -y curl

#==========================
# neovim
RUN wget https://github.com/neovim/neovim/releases/download/v0.9.5/nvim-linux64.tar.gz
RUN mkdir /home/nvim
RUN mv nvim-linux64.tar.gz /home/nvim
RUN cd /home/nvim; tar -xf nvim-linux64.tar.gz
RUN ln -s /home/nvim/nvim-linux64/bin/nvim /usr/bin/nvim

