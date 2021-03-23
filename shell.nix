let
  pkgs = import <nixpkgs> { };
  llvmPackages = pkgs.llvmPackages_11;
  stdenv = llvmPackages.stdenv;
  cmake = pkgs.cmake.overrideAttrs (
    oldAttrs: rec {
      inherit stdenv;
    }
  );
  cmake-format = cmake-format.override {
    inherit stdenv;
  };
  clang-analyzer = pkgs.clang-analyzer.override {
    clang = llvmPackages.clang;
    inherit stdenv;
    inherit llvmPackages;
  };
  clang-tools = pkgs.clang-tools.override {
    inherit stdenv;
    inherit llvmPackages;
  };
  cmakelint = pkgs.python3.pkgs.buildPythonApplication rec {
    pname = "cmakelint";
    version = "1.4";

    src = pkgs.python3.pkgs.fetchPypi {
      inherit pname version;
      sha256 = "0y79rsjmyih9iqrsgff0cwdl5pxvxbbczl604m8g0hr1rf0b8dlk";
    };

    doCheck = false;

    meta = {
      homepage = "https://github.com/richq/cmake-lint";
      description = "cmakelint parses CMake files and reports style issues.";
    };
  };
  gcc = pkgs.gcc10;
in
stdenv.mkDerivation {

  name = "reflection-dev";

  buildInputs = [
    # gcc
    cmake
    clang-analyzer
    cmakelint
    clang-tools
  ];

  CXX = "${clang-analyzer}/libexec/scan-build/c++-analyzer";
  CC = "${clang-analyzer}/libexec/scan-build/ccc-analyzer";

  # CXX = "${gcc}/bin/g++";
  # CCC = "${gcc}/bin/gcc";
}
