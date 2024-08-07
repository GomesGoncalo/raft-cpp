#include <doctest/doctest.h>

#include <array>
#include <exception>
#include <raftlib/raft_options.hxx>
#include <stdexcept>

struct MockYaml final {
  [[nodiscard]] MockYaml operator[](const std::string &) const noexcept {
    return MockYaml{};
  }

  template <typename Val>
  [[nodiscard]] Val as() const noexcept(noexcept(Val())) {
    return Val{};
  }
};

struct ExceptionYamlAccess final {
  [[nodiscard]] ExceptionYamlAccess operator[](const std::string &) const {
    throw std::runtime_error("does not exist");
  }

  template <typename Val>
  [[nodiscard]] Val as() const noexcept(noexcept(Val())) {
    return Val{};
  }
};

struct ExceptionYamlConvert final {
  [[nodiscard]] ExceptionYamlConvert
  operator[](const std::string &) const noexcept {
    return ExceptionYamlConvert{};
  }

  template <typename Val> [[nodiscard]] Val as() const {
    throw std::runtime_error("cannot convert");
    return Val{};
  }
};

struct MockReader final {
  [[nodiscard]] auto operator()(const std::string &) const noexcept {
    return MockYaml{};
  }
};

struct ThrowReader final {
  [[nodiscard]] MockYaml operator()(const std::string &) const {
    throw std::runtime_error("file does not exist");
    return MockYaml{};
  }
};

TEST_SUITE_BEGIN("raft options");

TEST_CASE(
    "raft options returns empty optional when not valid options are passed") {
  auto data = std::to_array<const char *>(
      {"raft", "--invalid_paramhopethisisnever a real parameter"});
  CHECK_THROWS_AS(
      auto _ = raft_options::create(data.size(), data.data(), MockReader{}),
      const std::exception &);
}

TEST_CASE("raft options returns empty optional when help is passed") {
  auto data = std::to_array<const char *>({"raft", "--help"});
  auto opt =
      raft_options::create<MockReader>(data.size(), data.data(), MockReader{});
  REQUIRE(!opt.has_value());
}

TEST_CASE(
    "raft options config file is passed but does not exist return nullopt") {
  auto data =
      std::to_array<const char *>({"raft", "--config", "~/notexistent"});
  CHECK_THROWS_AS(
      auto _ = raft_options::create(data.size(), data.data(), ThrowReader{}),
      const std::exception &);
}

TEST_CASE("raft options config file is passed and exists") {
  auto data = std::to_array<const char *>({"raft", "--config", "~/existent"});
  auto opt = raft_options::create(data.size(), data.data(), MockReader{});
  REQUIRE(opt.has_value());
}

TEST_CASE("if an exception is thrown whilst reading optional yaml field "
          "nothing happens") {
  ExceptionYamlAccess ex;
  std::string var = "stays the same";
  detail::get_yaml<true>(ex, var, "non", "existent", "key");
  REQUIRE(var == "stays the same");
}

TEST_CASE("if an exception is thrown whilst reading required yaml field "
          "rethrow") {
  ExceptionYamlAccess ex;
  std::string var = "stays the same";
  CHECK_THROWS_AS(detail::get_yaml<false>(ex, var, "non", "existent", "key"),
                  const std::exception &);
}

TEST_CASE("if an exception is thrown whilst converting optional yaml field "
          "nothing happens") {
  ExceptionYamlConvert ex;
  std::string var = "stays the same";
  detail::get_yaml<true>(ex, var, "non", "existent", "key");
  REQUIRE(var == "stays the same");
}

TEST_CASE("if an exception is thrown whilst converting required yaml field "
          "rethrow") {
  ExceptionYamlConvert ex;
  std::string var = "stays the same";
  CHECK_THROWS_AS(detail::get_yaml<false>(ex, var, "non", "existent", "key"),
                  const std::exception &);
}
TEST_SUITE_END();
