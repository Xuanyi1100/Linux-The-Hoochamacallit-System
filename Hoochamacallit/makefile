.PHONY: all clean install

all:
	$(MAKE) -C Common
	$(MAKE) -C DC
	$(MAKE) -C DR
	$(MAKE) -C DX
	$(MAKE) install

install:
	@cp DR/bin/DR Common/bin/
	@cp DX/bin/DX Common/bin/

clean:
	$(MAKE) -C Common clean
	$(MAKE) -C DC clean
	$(MAKE) -C DR clean
	$(MAKE) -C DX clean
	@rm -f Common/bin/*

