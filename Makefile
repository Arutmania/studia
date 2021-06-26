CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -g

macroprocessor: FORCE macroprocessor.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) macroprocessor.cpp -o macroprocessor

FORCE: ;
