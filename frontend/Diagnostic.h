#pragma once
#include <string>
#include <vector>
#include <ostream>

namespace mana::frontend {

    enum class DiagKind { Error, Warning, Note, Help };

    // Secondary location for multi-location diagnostics
    struct SecondarySpan {
        int line = 1;
        int column = 1;
        int span_length = 1;
        std::string label;
    };

    struct Diagnostic {
        DiagKind kind{};
        std::string message;
        int line = 1;
        int column = 1;
        int span_length = 1;  // Length of the error span for underlining

        // Enhanced fields
        std::string error_code;       // e.g., "E0001" for type mismatch
        std::string help_message;     // Additional help text
        std::string suggestion;       // Suggested fix
        std::vector<SecondarySpan> related;  // Related locations
    };

    class DiagnosticEngine {
    public:
        DiagnosticEngine() = default;

        void set_source(const std::string& filename, const std::string& source);

        // Color output control
        void set_color_enabled(bool enabled) { color_enabled_ = enabled; }
        bool color_enabled() const { return color_enabled_; }

        // Basic diagnostic methods
        void error(const std::string& msg, int line, int column, int span_length = 1);
        void warning(const std::string& msg, int line, int column, int span_length = 1);
        void note(const std::string& msg, int line, int column, int span_length = 1);
        void help(const std::string& msg, int line, int column, int span_length = 1);

        // Enhanced diagnostic methods with error codes and suggestions
        void error_with_code(const std::string& code, const std::string& msg,
                            int line, int column, int span_length = 1);
        void error_with_help(const std::string& msg, const std::string& help,
                            int line, int column, int span_length = 1);
        void error_with_suggestion(const std::string& msg, const std::string& suggestion,
                                  int line, int column, int span_length = 1);

        // Full diagnostic with all options
        void emit(DiagKind kind, const std::string& msg, int line, int column,
                 int span_length = 1, const std::string& code = "",
                 const std::string& help = "", const std::string& suggestion = "",
                 const std::vector<SecondarySpan>& related = {});

        // Add related span to last diagnostic
        void add_related(int line, int column, int span_length, const std::string& label);

        bool has_errors() const;
        bool has_warnings() const;
        bool has_any() const;

        // Counts
        size_t error_count() const;
        size_t warning_count() const;

        void print_all(std::ostream& out) const;
        void clear() { diags_.clear(); }

        // Accessors for tools
        const std::vector<Diagnostic>& all() const { return diags_; }
        std::vector<Diagnostic> errors() const {
            std::vector<Diagnostic> result;
            for (const auto& d : diags_) if (d.kind == DiagKind::Error) result.push_back(d);
            return result;
        }
        std::vector<Diagnostic> warnings() const {
            std::vector<Diagnostic> result;
            for (const auto& d : diags_) if (d.kind == DiagKind::Warning) result.push_back(d);
            return result;
        }

    private:
        std::vector<Diagnostic> diags_;
        std::string filename_;
        std::vector<std::string> source_lines_;
        bool color_enabled_ = true;  // Enable colors by default

        std::string get_line(int line_num) const;

        // ANSI color codes
        const char* color_reset() const { return color_enabled_ ? "\033[0m" : ""; }
        const char* color_bold() const { return color_enabled_ ? "\033[1m" : ""; }
        const char* color_red() const { return color_enabled_ ? "\033[31m" : ""; }
        const char* color_yellow() const { return color_enabled_ ? "\033[33m" : ""; }
        const char* color_blue() const { return color_enabled_ ? "\033[34m" : ""; }
        const char* color_cyan() const { return color_enabled_ ? "\033[36m" : ""; }
        const char* color_green() const { return color_enabled_ ? "\033[32m" : ""; }
        const char* color_magenta() const { return color_enabled_ ? "\033[35m" : ""; }

        void print_diagnostic(std::ostream& out, const Diagnostic& d) const;
        void print_source_line(std::ostream& out, int line_num, int highlight_col,
                              int span_length, bool is_primary) const;
    };

} // namespace mana::frontend
