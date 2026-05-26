BUILD_TYPE ?= Release

include utils.mk

all: zappy_server # zappy_gui

zappy_server:
	@ cmake -S server -B server/build -G Ninja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@ ninja -C server/build
	@ cp server/build/zappy_server ./zappy_server
	@ $(LOG_TIME) "$(C_BLUE) OK $(C_GREEN) server built $(C_RESET)"

zappy_gui:
	@ cmake -S gui -B gui/build -G Ninja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@ ninja -C gui/build
	@ cp server/build/zappy_gui ./zappy_gui
	@ $(LOG_TIME) "$(C_BLUE) OK $(C_GREEN) gui built $(C_RESET)"

debug:
	$(MAKE) BUILD_TYPE=Debug

format:
	@ find server/src gui/src -name "*.cpp" -o -name "*.hpp" 2>/dev/null | xargs -r clang-format -i
	@ $(LOG_TIME) "$(C_BLUE) CF $(C_GREEN) code formatted $(C_RESET)"

clean:
	@ rm -rf server/build gui/build
	@ $(LOG_TIME) "$(C_YELLOW) RM $(C_PURPLE) server/build gui/build $(C_RESET)"

fclean: clean
	@ rm -f zappy_server zappy_gui
	@ $(LOG_TIME) "$(C_YELLOW) RM $(C_PURPLE) zappy_server zappy_gui $(C_RESET)"

re: fclean all

.PHONY: all zappy_server zappy_gui debug format clean fclean re
