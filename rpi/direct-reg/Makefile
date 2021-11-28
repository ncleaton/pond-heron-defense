
ALL_BIN: /root/rpibin/*

/root/rpibin/%: %.c
	gcc -Wall -O3 -o $@.tmp $<
	mv -f $@.tmp $@

.PHONEY: ALL_BIN
