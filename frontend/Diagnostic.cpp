#include "Diagnostic.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace mana::frontend {

    static const char* kind_name(DiagKind k) {
        switch (k) {
        case DiagKind::Error: return "error";
        case DiagKind::Warning: return "warning";
        case DiagKind::Note: return "note";
        case DiagKind::Help: return "help";
        default: return "diag";
        }
    }

    void DiagnosticEngine::set_source(const std::string& filename, const std::string& source) {
        filename_ = filename;
        source_lines_.clear();

        std::istringstream stream(source);
        std::string line;
        while (std::getline(stream, line)) {
            source_lines_.push_back(line);
        }
    }

    std::string DiagnosticEngine::get_line(int line_num) const {
        if (line_num < 1 || line_num > static_cast<int>(source_lines_.size())) {
            return "";
        }
        return source_lines_[line_num - 1];
    }

    void DiagnosticEngine::error(const std::string& msg, int line, int column, int span_length) {
        Diagnostic d; d.kind = DiagKind::Error; d.message = msg; d.line = line; d.column = column; d.span_length = span_length; diags_.push_back(d);
    }

    void DiagnosticEngine::warning(const std::string& msg, int line, int column, int span_length) {
        Diagnostic d; d.kind = DiagKind::Warning; d.message = msg; d.line = line; d.column = column; d.span_length = span_length; diags_.push_back(d);
    }

    void DiagnosticEngine::note(const std::string& msg, int line, int column, int span_length) {
        Diagnostic d; d.kind = DiagKind::Note; d.message = msg; d.line = line; d.column = column; d.span_length = span_length; diags_.push_back(d);
    }


    void DiagnosticEngine::help(const std::string& msg, int line, int column, int span_length) {
        Diagnostic d; d.kind = DiagKind::Help; d.message = msg; d.line = line; d.column = column; d.span_length = span_length; diags_.push_back(d);
    }

    void DiagnosticEngine::error_with_code(const std::string& code, const std::string& msg, int line, int column, int span_length) {
        Diagnostic d; d.kind = DiagKind::Error; d.message = msg; d.line = line; d.column = column; d.span_length = span_length; d.error_code = code; diags_.push_back(d);
    }

    void DiagnosticEngine::error_with_help(const std::string& msg, const std::string& help, int line, int column, int span_length) {
        Diagnostic d; d.kind = DiagKind::Error; d.message = msg; d.line = line; d.column = column; d.span_length = span_length; d.help_message = help; diags_.push_back(d);
    }

    void DiagnosticEngine::error_with_suggestion(const std::string& msg, const std::string& suggestion, int line, int column, int span_length) {
        Diagnostic d; d.kind = DiagKind::Error; d.message = msg; d.line = line; d.column = column; d.span_length = span_length; d.suggestion = suggestion; diags_.push_back(d);
    }

    void DiagnosticEngine::emit(DiagKind kind, const std::string& msg, int line, int column, int span_length,
                               const std::string& code, const std::string& help, const std::string& suggestion,
                               const std::vector<SecondarySpan>& related) {
        Diagnostic d; d.kind = kind; d.message = msg; d.line = line; d.column = column; d.span_length = span_length;
        d.error_code = code; d.help_message = help; d.suggestion = suggestion; d.related = related; diags_.push_back(d);
    }

    void DiagnosticEngine::add_related(int line, int column, int span_length, const std::string& label) {
        if (!diags_.empty()) {
            SecondarySpan span; span.line = line; span.column = column; span.span_length = span_length; span.label = label;
            diags_.back().related.push_back(span);
        }
    }

    bool DiagnosticEngine::has_errors() const {
        for (const auto& d : diags_) if (d.kind == DiagKind::Error) return true;
        return false;
    }

    bool DiagnosticEngine::has_any() const {
        return !diags_.empty();
    }

    bool DiagnosticEngine::has_warnings() const {
        for (const auto& d : diags_) if (d.kind == DiagKind::Warning) return true;
        return false;
    }

    size_t DiagnosticEngine::error_count() const {
        size_t c = 0; for (const auto& d : diags_) if (d.kind == DiagKind::Error) ++c; return c;
    }

    size_t DiagnosticEngine::warning_count() const {
        size_t c = 0; for (const auto& d : diags_) if (d.kind == DiagKind::Warning) ++c; return c;
    }


    void DiagnosticEngine::print_all(std::ostream& out) const {
        for (const auto& d : diags_) {
            out << kind_name(d.kind) << ": " << d.message << "\n";
            if (!filename_.empty()) {
                out << "  --> " << filename_ << ":" << d.line << ":" << d.column << "\n";
            } else {
                out << "  --> line " << d.line << ", column " << d.column << "\n";
            }
            std::string src_line = get_line(d.line);
            if (!src_line.empty()) {
                int max_line = std::min(d.line + 1, static_cast<int>(source_lines_.size()));
                int line_width = static_cast<int>(std::to_string(max_line).length());
                if (line_width < 3) line_width = 3;
                if (d.line > 1) {
                    std::string ctx_before = get_line(d.line - 1);
                    if (!ctx_before.empty()) {
                        out << "   " << std::setw(line_width) << (d.line - 1) << " | " << ctx_before << "\n";
                    }
                }
                out << "   " << std::setw(line_width) << d.line << " | " << src_line << "\n";
                out << "   " << std::setw(line_width) << " " << " | ";
                for (int i = 1; i < d.column; ++i) {
                    if (i - 1 < static_cast<int>(src_line.length()) && src_line[i - 1] == '\t') {
                        out << "\t";
                    } else {
                        out << " ";
                    }
                }
                int span = d.span_length;
                if (span < 1) span = 1;
                int remaining = static_cast<int>(src_line.length()) - d.column + 1;
                if (span > remaining) span = remaining;
                if (span < 1) span = 1;
                out << "^";
                for (int i = 1; i < span; ++i) {
                    out << "~";
                }
                out << "\n";
                if (d.line < static_cast<int>(source_lines_.size())) {
                    std::string ctx_after = get_line(d.line + 1);
                    if (!ctx_after.empty()) {
                        out << "   " << std::setw(line_width) << (d.line + 1) << " | " << ctx_after << "\n";
                    }
                }
            }
            out << "\n";
        }
    }

} // namespace mana::frontend
