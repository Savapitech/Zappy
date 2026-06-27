BUILD_TYPE ?= Release

include utils.mk

all: zappy_server zappy_gui zappy_ai

zappy_ai:
	@ python3 ia/compile.py
	@ $(LOG_TIME) "$(C_BLUE) OK $(C_GREEN) ai built $(C_RESET)"

zappy_server: check_vcpkg
	@ cmake -S server -B server/build -G Ninja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
	@ ninja -C server/build
	@ cp server/build/zappy_server ./zappy_server
	@ cp server/build/compile_commands.json ./
	@ $(LOG_TIME) "$(C_BLUE) OK $(C_GREEN) server built $(C_RESET)"

zappy_gui: check_vcpkg
	@ cmake -S gui -B gui/build -G Ninja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
	@ ninja -C gui/build
	@ cp gui/build/zappy_gui ./zappy_gui
	@ $(LOG_TIME) "$(C_BLUE) OK $(C_GREEN) gui built $(C_RESET)"

check_vcpkg:
	@ test -n "$(VCPKG_ROOT)" || (echo "$(C_RED)Error:$(C_RESET) VCPKG_ROOT is not set. Install vcpkg and export VCPKG_ROOT=/path/to/vcpkg" && exit 1)

debug:
	$(MAKE) BUILD_TYPE=Debug

hooks:
	@ git config core.hooksPath .githooks
	@ $(LOG_TIME) "$(C_BLUE) OK $(C_GREEN) git hooks installed $(C_RESET)"

format:
	@ find server/src gui/src -name "*.cpp" -o -name "*.hpp" 2>/dev/null | xargs -r clang-format -i
	@ $(LOG_TIME) "$(C_BLUE) CF $(C_GREEN) code formatted $(C_RESET)"

tests_unit_server: check_vcpkg
	@ cmake -S server -B server/build-tests -G Ninja -DBUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
	@ ninja -C server/build-tests
	@ ./server/build-tests/zappy_tests
	@ $(LOG_TIME) "$(C_BLUE) OK $(C_GREEN) unit tests passed $(C_RESET)"

tests_func_server: zappy_server
	@ test -d server/tests/.venv || python3 -m venv server/tests/.venv
	@ server/tests/.venv/bin/pip install -q --upgrade pip pytest
	@ cd server/tests/functional && ../.venv/bin/python -m pytest
	@ $(LOG_TIME) "$(C_BLUE) OK $(C_GREEN) functional tests passed $(C_RESET)"

tests_run: tests_unit_server tests_func_server
	@ $(LOG_TIME) "$(C_BLUE) OK $(C_GREEN) all tests passed $(C_RESET)"

clean:
	@ rm -rf server/build gui/build
	@ $(LOG_TIME) "$(C_YELLOW) RM $(C_PURPLE) server/build gui/build $(C_RESET)"

fclean: clean
	@ rm -f zappy_server zappy_gui zappy_ai
	@ $(LOG_TIME) "$(C_YELLOW) RM $(C_PURPLE) zappy_server zappy_gui zappy_ai $(C_RESET)"

re: fclean all

.PHONY: all zappy_server zappy_gui zappy_ai check_vcpkg debug hooks format clean fclean re \
	tests_unit_server tests_func_server tests_run
