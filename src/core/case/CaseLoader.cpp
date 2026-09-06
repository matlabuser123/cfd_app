#include "core/case/CaseLoader.hpp"

#include "core/case/CaseValidator.hpp"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <utility>

namespace cfd
{
namespace
{

struct Json;
using JsonArray = std::vector<Json>;
using JsonObject = std::map<std::string, Json>;

struct Json : std::variant<std::nullptr_t, bool, double, std::string, JsonArray, JsonObject>
{
    using std::variant<std::nullptr_t, bool, double, std::string, JsonArray, JsonObject>::variant;
};

class JsonParser
{
public:
    explicit JsonParser(std::string text) : text_(std::move(text)) {}

    Json parse()
    {
        skipWhitespace();
        Json value = parseValue();
        skipWhitespace();
        if (position_ != text_.size()) throw std::invalid_argument("Unexpected JSON content");
        return value;
    }

private:
    void skipWhitespace() { while (position_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[position_]))) ++position_; }
    char consume()
    {
        if (position_ >= text_.size()) throw std::invalid_argument("Unexpected end of JSON");
        return text_[position_++];
    }
    void expect(char expected) { if (consume() != expected) throw std::invalid_argument("Malformed JSON"); }

    Json parseValue()
    {
        skipWhitespace();
        if (position_ >= text_.size()) throw std::invalid_argument("Missing JSON value");
        switch (text_[position_])
        {
        case '{': return parseObject();
        case '[': return parseArray();
        case '"': return parseString();
        case 't': return parseLiteral("true", true);
        case 'f': return parseLiteral("false", false);
        case 'n': return parseLiteral("null", nullptr);
        default: return parseNumber();
        }
    }

    Json parseObject()
    {
        std::map<std::string, Json> object;
        expect('{'); skipWhitespace();
        if (position_ < text_.size() && text_[position_] == '}') { ++position_; return object; }
        while (true)
        {
            skipWhitespace();
            const std::string key = std::get<std::string>(parseString());
            skipWhitespace(); expect(':'); object.emplace(key, parseValue()); skipWhitespace();
            const char delimiter = consume();
            if (delimiter == '}') break;
            if (delimiter != ',') throw std::invalid_argument("Malformed JSON object");
        }
        return object;
    }

    Json parseArray()
    {
        std::vector<Json> array;
        expect('['); skipWhitespace();
        if (position_ < text_.size() && text_[position_] == ']') { ++position_; return array; }
        while (true)
        {
            array.push_back(parseValue()); skipWhitespace();
            const char delimiter = consume();
            if (delimiter == ']') break;
            if (delimiter != ',') throw std::invalid_argument("Malformed JSON array");
        }
        return array;
    }

    Json parseString()
    {
        expect('"'); std::string result;
        while (position_ < text_.size())
        {
            const char value = consume();
            if (value == '"') return result;
            if (value == '\\')
            {
                const char escaped = consume();
                if (escaped == '"' || escaped == '\\' || escaped == '/') result += escaped;
                else if (escaped == 'n') result += '\n';
                else if (escaped == 't') result += '\t';
                else throw std::invalid_argument("Unsupported JSON escape");
            }
            else result += value;
        }
        throw std::invalid_argument("Unterminated JSON string");
    }

    template <typename T>
    Json parseLiteral(const std::string& literal, T value)
    {
        if (text_.compare(position_, literal.size(), literal) != 0) throw std::invalid_argument("Malformed JSON literal");
        position_ += literal.size(); return value;
    }

    Json parseNumber()
    {
        const std::size_t start = position_;
        while (position_ < text_.size() && (std::isdigit(static_cast<unsigned char>(text_[position_])) || text_[position_] == '-' || text_[position_] == '+' || text_[position_] == '.' || text_[position_] == 'e' || text_[position_] == 'E')) ++position_;
        try { return std::stod(text_.substr(start, position_ - start)); }
        catch (...) { throw std::invalid_argument("Malformed JSON number"); }
    }

    std::string text_;
    std::size_t position_{0};
};

using Object = std::map<std::string, Json>;
const Object& object(const Json& value) { return std::get<Object>(value); }
const Json& required(const Object& value, const std::string& key) { const auto it = value.find(key); if (it == value.end()) throw std::invalid_argument("Missing required key: " + key); return it->second; }
std::string stringValue(const Json& value) { return std::get<std::string>(value); }
double numberValue(const Json& value) { return std::get<double>(value); }
bool boolValue(const Json& value) { return std::get<bool>(value); }
Vector2 vectorValue(const Json& value)
{
    const auto& values = std::get<JsonArray>(value);
    if (values.size() != 2) throw std::invalid_argument("Expected a two-dimensional vector");
    return {numberValue(values[0]), numberValue(values[1])};
}
Json readJson(const std::filesystem::path& path)
{
    std::ifstream stream(path);
    if (!stream) throw std::runtime_error("Unable to open case file: " + path.string());
    std::ostringstream content; content << stream.rdbuf();
    return JsonParser(content.str()).parse();
}

}

Case CaseLoader::load(const std::filesystem::path& directory) const
{
    const Object root = object(readJson(directory / "case.json"));
    CaseConfig config;
    config.name = stringValue(required(root, "name"));
    config.dimensions = static_cast<int>(numberValue(required(root, "dimensions")));

    const auto loadPath = [&](const std::string& key) { return directory / stringValue(required(root, key)); };
    const Object mesh = object(readJson(loadPath("mesh")));
    config.mesh.nx = static_cast<std::size_t>(numberValue(required(mesh, "nx")));
    config.mesh.ny = static_cast<std::size_t>(numberValue(required(mesh, "ny")));
    config.mesh.width = numberValue(required(mesh, "width"));
    config.mesh.height = numberValue(required(mesh, "height"));

    const Object physics = object(readJson(loadPath("physics")));
    config.fluid.name = stringValue(required(physics, "fluid"));
    config.fluid.density = numberValue(required(physics, "density"));
    config.fluid.dynamicViscosity = numberValue(required(physics, "dynamic_viscosity"));
    config.fluid.gravity = vectorValue(required(physics, "gravity"));
    config.fluid.compressible = boolValue(required(physics, "compressible"));

    const Object boundaries = object(required(object(readJson(loadPath("boundary_conditions"))), "boundaries"));
    for (const auto& [patch, raw] : boundaries)
    {
        const Object value = object(raw); BoundarySpec spec; spec.type = stringValue(required(value, "type"));
        if (const auto it = value.find("value"); it != value.end())
        {
            if (std::holds_alternative<std::vector<Json>>(it->second)) spec.vectorValue = vectorValue(it->second); else spec.value = numberValue(it->second);
        }
        if (const auto it = value.find("gradient"); it != value.end()) spec.gradient = numberValue(it->second);
        config.boundaries.emplace(patch, spec);
    }

    const Object initial = object(readJson(loadPath("initial_conditions")));
    config.initialConditions.velocity = vectorValue(required(initial, "velocity"));
    config.initialConditions.pressure = numberValue(required(initial, "pressure"));

    const Object numerics = object(readJson(loadPath("numerics")));
    config.numerics.algorithm = stringValue(required(numerics, "algorithm"));
    config.numerics.momentumSolver = stringValue(required(numerics, "momentum_solver"));
    config.numerics.pressureSolver = stringValue(required(numerics, "pressure_solver"));
    config.numerics.momentumTolerance = numberValue(required(numerics, "momentum_tolerance"));
    config.numerics.pressureTolerance = numberValue(required(numerics, "pressure_tolerance"));
    config.numerics.maxIterations = static_cast<std::size_t>(numberValue(required(numerics, "max_iterations")));
    const Object relaxation = object(required(numerics, "under_relaxation"));
    config.numerics.velocityUnderRelaxation = numberValue(required(relaxation, "velocity"));
    config.numerics.pressureUnderRelaxation = numberValue(required(relaxation, "pressure"));

    const Object output = object(readJson(loadPath("output")));
    config.output.directory = stringValue(required(output, "directory"));
    config.output.writeCsv = boolValue(required(output, "write_csv"));
    config.output.writeJson = boolValue(required(output, "write_json"));
    config.output.writeVtk = boolValue(required(output, "write_vtk"));

    CaseValidator::validate(config);
    return Case(std::move(config));
}

}
