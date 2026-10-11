MAX_LINE_LENGTH = 120

SYSTEM := $(shell uname -s)

black:
	@black python/ --line-length $(MAX_LINE_LENGTH)
	@black extrapolation.py --line-length $(MAX_LINE_LENGTH)

isort:
	@isort python/ --line-length $(MAX_LINE_LENGTH) --multi-line 3 --profile black
	@isort extrapolation.py --line-length $(MAX_LINE_LENGTH) --multi-line 3 --profile black

flake8:
	@flake8 python/ --exclude venv/,testing/memory_profiling/ --max-line-length $(MAX_LINE_LENGTH) --ignore=E203,W503
	@flake8 extrapolation.py --exclude venv/,testing/memory_profiling/ --max-line-length $(MAX_LINE_LENGTH) --ignore=E203,W503

pyformat:
	@make black
	@make isort
	@make flake8

BRUTE_FORCE_DIR = cpp_brute_force
TRANSFER_MATRIX_DIR = cpp_transfer_matrix

MACOS_FLAGS = -O3 -march=native -ffast-math -std=c++17
LINUX_FLAGS = -O3 -march=native -stdlib=libc++ -ffast-math -std=c++17

cppformat:
	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_combinations_no_border.cpp
	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_combinations_with_border.cpp

	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_counting_utils.cpp
	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_counting_utils.h

	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_no_border_utils.cpp
	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_no_border_utils.h

	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_permutations_no_border.cpp
	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_permutations_with_border.cpp

	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_with_border_utils.cpp
	@clang-format -i --style Mozilla $(BRUTE_FORCE_DIR)/board_game_with_border_utils.h

	@clang-format -i --style Mozilla $(TRANSFER_MATRIX_DIR)/board_game_combinations_no_border.cpp
	@clang-format -i --style Mozilla $(TRANSFER_MATRIX_DIR)/board_game_combinations_with_border.cpp

	@clang-format -i --style Mozilla $(TRANSFER_MATRIX_DIR)/board_game_permutations_no_border.cpp
	@clang-format -i --style Mozilla $(TRANSFER_MATRIX_DIR)/board_game_permutations_with_border.cpp

	@clang-format -i --style Mozilla $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.cpp
	@clang-format -i --style Mozilla $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.h

compile:
	@make compile_brute_force
	@make compile_transfer_matrix

compile_brute_force:
ifeq ($(SYSTEM), Darwin)
	@echo 'Compiling brute force on MacOS'
	@g++ $(MACOS_FLAGS) $(BRUTE_FORCE_DIR)/board_game_counting_utils.cpp $(BRUTE_FORCE_DIR)/board_game_no_border_utils.cpp $(BRUTE_FORCE_DIR)/board_game_combinations_no_border.cpp -o $(BRUTE_FORCE_DIR)/board_game_combinations_no_border
	@g++ $(MACOS_FLAGS) $(BRUTE_FORCE_DIR)/board_game_counting_utils.cpp $(BRUTE_FORCE_DIR)/board_game_with_border_utils.cpp $(BRUTE_FORCE_DIR)/board_game_combinations_with_border.cpp -o $(BRUTE_FORCE_DIR)/board_game_combinations_with_border
	@g++ $(MACOS_FLAGS) $(BRUTE_FORCE_DIR)/board_game_counting_utils.cpp $(BRUTE_FORCE_DIR)/board_game_no_border_utils.cpp $(BRUTE_FORCE_DIR)/board_game_permutations_no_border.cpp -o $(BRUTE_FORCE_DIR)/board_game_permutations_no_border
	@g++ $(MACOS_FLAGS) $(BRUTE_FORCE_DIR)/board_game_counting_utils.cpp $(BRUTE_FORCE_DIR)/board_game_with_border_utils.cpp $(BRUTE_FORCE_DIR)/board_game_permutations_with_border.cpp -o $(BRUTE_FORCE_DIR)/board_game_permutations_with_border
else ifeq ($(SYSTEM), Linux)
	@echo "Compiling brute force on Linux"
	@clang++ $(LINUX_FLAGS) $(BRUTE_FORCE_DIR)/board_game_counting_utils.cpp $(BRUTE_FORCE_DIR)/board_game_no_border_utils.cpp $(BRUTE_FORCE_DIR)/board_game_combinations_no_border.cpp -o $(BRUTE_FORCE_DIR)/board_game_combinations_no_border
	@clang++ $(LINUX_FLAGS) $(BRUTE_FORCE_DIR)/board_game_counting_utils.cpp $(BRUTE_FORCE_DIR)/board_game_with_border_utils.cpp $(BRUTE_FORCE_DIR)/board_game_combinations_with_border.cpp -o $(BRUTE_FORCE_DIR)/board_game_combinations_with_border
	@clang++ $(LINUX_FLAGS) $(BRUTE_FORCE_DIR)/board_game_counting_utils.cpp $(BRUTE_FORCE_DIR)/board_game_no_border_utils.cpp $(BRUTE_FORCE_DIR)/board_game_permutations_no_border.cpp -o $(BRUTE_FORCE_DIR)/board_game_permutations_no_border
	@clang++ $(LINUX_FLAGS) $(BRUTE_FORCE_DIR)/board_game_counting_utils.cpp $(BRUTE_FORCE_DIR)/board_game_with_border_utils.cpp $(BRUTE_FORCE_DIR)/board_game_permutations_with_border.cpp -o $(BRUTE_FORCE_DIR)/board_game_permutations_with_border
else
	@echo "Unsupported operating system $(SYSTEM)"
endif

compile_transfer_matrix:
ifeq ($(SYSTEM), Darwin)
	@echo 'Compiling transfer matrix on MacOS'
	@g++ $(MACOS_FLAGS) $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.cpp $(TRANSFER_MATRIX_DIR)/board_game_combinations_no_border.cpp -o $(TRANSFER_MATRIX_DIR)/board_game_combinations_no_border
	@g++ $(MACOS_FLAGS) $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.cpp $(TRANSFER_MATRIX_DIR)/board_game_combinations_with_border.cpp -o $(TRANSFER_MATRIX_DIR)/board_game_combinations_with_border
	@g++ $(MACOS_FLAGS) $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.cpp $(TRANSFER_MATRIX_DIR)/board_game_permutations_no_border.cpp -o $(TRANSFER_MATRIX_DIR)/board_game_permutations_no_border
	@g++ $(MACOS_FLAGS) $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.cpp $(TRANSFER_MATRIX_DIR)/board_game_permutations_with_border.cpp -o $(TRANSFER_MATRIX_DIR)/board_game_permutations_with_border
else ifeq ($(SYSTEM), Linux)
	@echo "Compiling transfer matrix on Linux"
	@clang++ $(LINUX_FLAGS) $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.cpp $(TRANSFER_MATRIX_DIR)/board_game_combinations_no_border.cpp -o $(TRANSFER_MATRIX_DIR)/board_game_combinations_no_border
	@clang++ $(LINUX_FLAGS) $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.cpp $(TRANSFER_MATRIX_DIR)/board_game_combinations_with_border.cpp -o $(TRANSFER_MATRIX_DIR)/board_game_combinations_with_border
	@clang++ $(LINUX_FLAGS) $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.cpp $(TRANSFER_MATRIX_DIR)/board_game_permutations_no_border.cpp -o $(TRANSFER_MATRIX_DIR)/board_game_permutations_no_border
	@clang++ $(LINUX_FLAGS) $(TRANSFER_MATRIX_DIR)/board_game_transfer_matrix_utils.cpp $(TRANSFER_MATRIX_DIR)/board_game_permutations_with_border.cpp -o $(TRANSFER_MATRIX_DIR)/board_game_permutations_with_border
else
	@echo "Unsupported operating system $(SYSTEM)"
endif

run:
	@./$(BRUTE_FORCE_DIR)/board_game_permutations_with_border -r 3 -c 3 -n 1

run_transfer_matrix:
	@./$(TRANSFER_MATRIX_DIR)/board_game_permutations_no_border -r 10 -c 10
