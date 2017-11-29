vlc-wrapper -I rc v4l2:///dev/video0:width=320:height=240:fps=15 --sout="#transcode{vcodec=mp4v}:std{access=http,mux=ts,dst=:8090/stream}"
