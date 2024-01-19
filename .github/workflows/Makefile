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

all: linux.yml zephyr.yml

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

clean:
	rm -f linux.yml zephyr.yml
