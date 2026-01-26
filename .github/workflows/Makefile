LINUX_DEPS=\
	make-workflow \
	head \
	variants-cmake \
	variants \
	steps-head \
	steps-cmake \
	steps-minsize \
	steps-release

all: linux.yml zephyr.yml native.yml

linux.yml: $(LINUX_DEPS)
	./make-workflow > $@

ZEPHYR_DEPS=\
	make-workflow-zephyr \
	head-zephyr \
	variants \
	steps-head \
	steps-minsize \
	steps-release

zephyr.yml: $(ZEPHYR_DEPS)
	./make-workflow-zephyr > $@

NATIVE_DEPS=\
	make-workflow \
	head-native \
	variants \
	steps-head \
	steps-minsize \
	steps-release

native.yml: $(NATIVE_DEPS)
	./make-workflow-native > $@

COREBOOT_DEPS=\
	make-workflow-coreboot \
	head-coreboot \
	variants \
	steps-head \
	steps-minsize \
	steps-release

coreboot.yml: $(COREBOOT_DEPS)
	./make-workflow-coreboot > $@

clean:
	rm -f linux.yml zephyr.yml coreboot.yml
