let
  pkgs = import <nixpkgs> {};
  llvmPackages = pkgs.llvmPackages_10;
  stdenv = llvmPackages.stdenv;
  cmake = pkgs.cmake.overrideAttrs (
    oldAttrs: rec {
      inherit stdenv;
      version = "3.17.3";
      src = pkgs.fetchurl {
        url = "${oldAttrs.meta.homepage}files/v${pkgs.lib.versions.majorMinor version}/cmake-${version}.tar.gz";
        # compare with https://cmake.org/files/v${lib.versions.majorMinor version}/cmake-${version}-SHA-256.txt
        sha256 = "0h4c3nwk7wmzcmmlwyb16zmjqr44l4k591m2y9p9zp3m498hvmhb";
      };
    }
  );
  cmake-format = cmake-format.override {
    inherit stdenv;
  };
  clang-analyzer = pkgs.clang-analyzer.override { clang = llvmPackages.clang; inherit stdenv; inherit llvmPackages; };
  clang-tools = pkgs.clang-tools.override { inherit stdenv; inherit llvmPackages; };
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
in
pkgs.mkShell {
  inherit stdenv;
  buildInputs = [
    cmake
    clang-analyzer
    cmakelint
    clang-tools
  ];

  CXX = "${clang-analyzer}/libexec/scan-build/c++-analyzer";
  CC = "${clang-analyzer}/libexec/scan-build/ccc-analyzer";
}
