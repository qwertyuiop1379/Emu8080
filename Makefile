TARGET = emu8080
RUN_ARGS = .

CXX = clang++
CFLAGS = -g -Wall -std=c++11 -I/opt/homebrew/include

CPP_FILES = $(wildcard *.cpp)
OBJS = $(foreach CPP_FILE,$(CPP_FILES),$(subst .cpp,.o,$(CPP_FILE)))

clean:
	@echo =\> Cleaning...
	@rm -f build/obj/*.o
	@rm -f build/$(TARGET)

stage:
	@mkdir -p build/obj

%.o: %.cpp
	@echo =\> Compiling $<...
	@$(CXX) -c $< -o build/obj/$@ $(CFLAGS)

target: stage $(OBJS)
	@echo =\> Compiling $(TARGET)...
	@$(CXX) $(wildcard build/obj/*.o) -o build/$(TARGET) $(CFLAGS)

run:
	@echo =\> Running $(TARGET)...
	@./build/$(TARGET) $(RUN_ARGS)

do: target run