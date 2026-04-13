all:
	$(MAKE) -C c54
	$(MAKE) -C c55

install:
	$(MAKE) -C c54 install
	$(MAKE) -C c55 install

clean:
	$(MAKE) -C c54 clean
	$(MAKE) -C c55 clean

.PHONY: all install clean
