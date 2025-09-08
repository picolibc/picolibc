LINUX_DEPS=\
	make-workflow \
	head \
	variants-cmake \
	variants \
	targets-cmake-linux \
	steps-head \
	steps-cmake \
	steps-minsize \
	steps-release

all: linux.yml zephyr.yml # coreboot.yml

linux.yml: $(LINUX_DEPS)
	./make-workflow > $@

ZEPHYR_DEPS=\
	make-workflow-zephyr \
	head-zephyr \
	variants \
	targets-zephyr \
	steps-head \
	steps-minsize \
	steps-release

zephyr.yml: $(ZEPHYR_DEPS)
	./make-workflow-zephyr > $@

COREBOOT_DEPS=\
	make-workflow-coreboot \
	head-coreboot \
	variants \
	targets-coreboot \
	steps-head \
	steps-minsize \
	steps-release

coreboot.yml: $(COREBOOT_DEPS)
	./make-workflow-coreboot > $@

clean:
	rm -f linux.yml zephyr.yml coreboot.yml
