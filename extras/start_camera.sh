sudo modprobe bcm2835-v4l2	
cvlc v4l2:///dev/video0:width=640:height=480:fps=15 --v4l2-vflip 1 --v4l2-hflip 1 --sout="#transcode{vcodec=mp4v}:std{access=http,mux=ts,dst=:8090/stream}"
