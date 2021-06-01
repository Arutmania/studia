#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

class Switch {
public:
    Switch(std::string filename, std::istream& is, std::ostream& os)
        : is(is)
        , os(os)
        , position(std::move(filename))
    {}

    void run() try {
        char c;
        while (os && get(c)) {
            if (c == '#')
                pound();
            else
                os << c;
        }

        if (os.fail())
            throw Exception { Error::OS_FAIL };

    } catch (Exception const& ex) {
        Error::log(*this, ex.what());
        std::exit(ex.error.code);
    }

    struct Error {
        enum Code {
            IS_FAIL = 1,
            OS_FAIL,
            SOURCE_FILE_MISSING,
            TARGET_FILE_EXISTS,
            POUND_FREE_TEXT,
            POUND_MACRO_BODY,
            MACRO_MISSING_NAME,
            MACRO_MISSING_DEFINITION,
            CORRESPONDING_MDEF,
            OTHER,
        } code;
        char const* const message;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc99-designator"
        static constexpr char const* MESSAGES[] = {
            [IS_FAIL] =
                "input stream error has occurred",
            [OS_FAIL] =
                "output stream error has occurred",
            [SOURCE_FILE_MISSING] =
                "source file does not exist",
            [TARGET_FILE_EXISTS] =
                "target file exists",
            [POUND_FREE_TEXT] =
                "# should be followed by #, MDEF, or MCALL",
            [POUND_MACRO_BODY] =
                "# in macro body should be followed by #, or MEND",
            [MACRO_MISSING_NAME] =
                "missing macro name",
            [MACRO_MISSING_DEFINITION] =
                "missing macro definition",
            [CORRESPONDING_MDEF] =
                "#MEND without corresponding #MDEF",
        };
#pragma GCC diagnostic pop

        Error(Code code)
            : code(code)
            , message(MESSAGES[code])
        {}

        Error(char const* message)
            : code(OTHER)
            , message(message)
        {}

        static void log(Switch& sw, Code code) {
            log(sw, MESSAGES[code]);
        }

        template<class... Args>
        static void log(Switch& sw, Args&&... args) {
            (
                (std::cerr << sw.position << ' ')
                << ...
                << std::forward<decltype(args)>(args)
            ) << '\n';
        }
    };

    struct Exception : std::exception {
        Error error;
        Exception(Error::Code code) : error(code) {}

        char const* what() const noexcept override {
            return error.message;
        }
    };

private:
    struct Position {
        Position(std::string filename) : filename(std::move(filename)) {}
        std::string filename;
        int line = 1, column = 0;

        std::istream& get(std::istream& is, char& c) {
            is.get(c);
            if (c == '\n') {
                line += 1;
                column = 0;
            } else {
                column += 1;
            }
            return is;
        }

        friend std::ostream& operator <<(std::ostream& os,
                                         Position const& pos) {
            return os
                << pos.filename
                << ':'
                << pos.line
                << ':'
                << pos.column;
        }
    };

    class Arguments {
        std::map<std::string, std::string> _;

    public:
        static Arguments read(Switch& sw) {
            auto arguments = Arguments {};
            auto& is = sw.is;
            char c;

            while (is.peek() != '\n') {
                while (std::isspace(is.peek()) && sw.get(c) && c != '\n')
                    continue;

                auto argument = sw.get_word();
                auto position = argument.find("=");

                arguments._.insert_or_assign(
                    argument.substr(0, position),
                    argument.substr(position + 1)
                );
            }

            // ignore following newline
            is.get(c);

            return arguments;
        }

        bool contains(std::string const& key) const {
            return _.find(key) != _.end();
        }

        std::string get(Switch& sw, std::string const& key) const {
            if (contains(key)) {
                return _.at(key);
            } else {
                Error::log(sw, "missing parameter '", key, "'");
                return "";
            }
        }

        friend std::ostream& operator <<(std::ostream& os, Arguments const& args) {
            auto const* delim = "";

            for (auto const& [key, value] : args._)
                os << key << ": " << value << std::exchange(delim, ", ");

            return os << '\n';
        }
    };

    struct Macro {
        std::string name, body;

        static Macro read(Switch& sw) {
            return Macro {
                read_name(sw),
                read_body(sw)
            };
        }

        void expand(Switch& sw, Arguments const& args) {
            auto ss = std::stringstream { body };
            char c;

             while (sw.os && ss.get(c)) {
                 if (c != '$') {
                     sw.os.put(c);
                 } else {
                     auto parameter = std::string {};

                     while (!std::isspace(ss.peek()) && ss.get(c))
                         parameter.push_back(c);

                     // TODO: change $parameter doesn't have to preceeded by whitespace
                     if (parameter == "$")
                         sw.os.put('$');
                     else if (parameter == "__name")
                         sw.os << name;
                     else
                         sw.os << args.get(sw, parameter);
                 }
             }
        }

        static std::string read_name(Switch& sw) {
            sw.ignore_whitespace();
            auto word = sw.get_word();
            if (word == "#MEND")
                throw Exception { Error::MACRO_MISSING_NAME };
            return word;
        }

        static std::string read_body(Switch& sw) {
            // TODO: change to the document
            // macro name must be followed by newline, or rather any whitespace,
            // and newline. after newline macro body begins

            auto body = std::string {};
            char c;

            auto& is = sw.is;

            // ignore any leading whitespace, newline begins macro body
            while (std::isspace(is.peek()) && sw.get(c))
                if (c == '\n')
                    break;

            while (sw.get(c)) {
                // TODO: change # is special inside macro body as well
                if (c != '#') {
                    body.push_back(c);
                } else {
                    auto command = sw.get_word();

                    if (command == "#")
                        body.push_back('#');
                    else if (command == "MEND")
                        break;
                    else
                        Error::log(sw, Error::POUND_MACRO_BODY);
                }
            }

            // ignore trainling newline (?)
            sw.get(c);

            return body;
        }
    };


    std::istream& is;
    std::ostream& os;

    class Library {
        std::unordered_map<std::string, std::string> _;

    public:
        bool contains(std::string const& name) const {
            return _.find(name) != _.end();
        }

        void add(Switch& sw, Macro macro) {
            if (contains(macro.name))
                Error::log(sw, "overwriting macro '", macro.name, "'");
            _.insert_or_assign(std::move(macro.name), std::move(macro.body));
        }

        Macro get(Switch& sw, std::string const& name) const {
            if (contains(name)) {
                return Macro { name, _.at(name) };
            } else {
                Error::log(sw, "missing macro definition: '", name, "'");
                return Macro { name, "" };
            }
        }

    } library = {};

    Position position;

    void pound() {
        auto command = get_word();

        if (command == "#")
            os << '#';
        else if (command == "MDEF")
            macro_def();
        else if (command == "MCALL")
            macro_call();
        else if (command == "MEND")
            Error::log(*this, Error::CORRESPONDING_MDEF);
        else
            Error::log(*this, Error::POUND_FREE_TEXT);
    }

    void macro_def() try {
        library.add(*this, Macro::read(*this));
    } catch (Exception const& ex) {
        Error::log(*this, ex.what());
    }

    void macro_call() {
        library
            .get(*this, Macro::read_name(*this))
            .expand(*this, Arguments::read(*this));
    }

    bool get(char& c) {
        position.get(is, c);

        if (is.fail() && !is.eof())
            throw Exception { Error::IS_FAIL };

        return is.good();
    }

    void ignore_whitespace() {
        char c;
        while (std::isspace(is.peek()) && get(c))
            continue;
    }

    std::string get_word() {
        char c;
        auto word = std::string {};

        while (!std::isspace(is.peek()) && get(c))
            word.push_back(c);

        return word;
    }
};


int main (int argc, char* argv[]) try {
    if (argc > 2) {
        if (std::filesystem::exists(argv[1]))
            throw Switch::Exception { Switch::Error::SOURCE_FILE_MISSING };

        if (!std::filesystem::exists(argv[2]))
            throw Switch::Exception { Switch::Error::TARGET_FILE_EXISTS };

        auto is = std::ifstream(argv[1]);
        auto os = std::ofstream(argv[2]);
        Switch(argv[1], is, os).run();
    } else if (argc > 1) {
        if (std::filesystem::exists(argv[1]))
            throw Switch::Exception { Switch::Error::SOURCE_FILE_MISSING };

        auto is = std::ifstream(argv[1]);
        Switch(argv[1], is, std::cout).run();
    } else {
        Switch("STDIN", std::cin, std::cout).run();
    }
} catch (Switch::Exception const& ex) {
    std::cerr << ex.what() << std::endl;
    std::exit(ex.error.code);
}
