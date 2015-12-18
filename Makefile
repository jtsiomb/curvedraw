PREFIX = /usr/local

src = $(wildcard src/*.cc)
obj = $(src:.cc=.o)
dep = $(obj:.o=.d)
bin = curvedraw

CXXFLAGS = -pedantic -Wall -g
LDFLAGS = $(libgl) -lvmath -ldrawtext -lm

sys := $(shell uname -s | sed 's/MINGW.*/win32/')

ifeq ($(sys), Darwin)
	libgl = -framework OpenGL -framework GLUT -lGLEW
else ifeq ($(sys), win32)
	libgl = -lopengl32 -lglut32 -lglew32
else
	libgl = -lGL -lglut -lGLEW
endif

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.cc
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@


.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(bin)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bin)
