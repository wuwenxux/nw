nw_cli.o: nw_cli.h manage.h utils.h nw_err.h
utils.o: utils.h
nw_rw_ioctl.o: nw_cli.h manage.h utils.h nw_err.h
nw_other_cmd.o: utils.h nw_cli.h manage.h
nw_peer_cmd.o: utils.h nw_cli.h manage.h nw_err.h
