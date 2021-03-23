{ nixpkgs ? <nixpkgs>
, system ? builtins.currentSystem
}:
let
  pkgs = import nixpkgs { };
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
  misc-nix = import
    (pkgs.fetchFromGitHub {
      owner = "grenewode";
      repo = "misc-nix";
      rev = "trunk";
      hash = "sha256:03cydq4p9w6c6i75m9dpdgkin4abr34v0xjaxyq4fir741xvdpgh";
    })
    { inherit nixpkgs system; };
  compdb =
    with pkgs.python3Packages;
    buildPythonApplication rec {
      pname = "compdb";
      version = "0.2.0";

      src = fetchPypi {
        inherit pname version;

        hash = "sha256:1pbr5f8b3mvcf7sksa7f4ql9q2ag046lwfpv2v46jphys2ninivk";
      };

      doCheck = false;
    };
in
stdenv.mkDerivation {

  name = "reflection-dev";

  buildInputs = [
    # gcc
    cmake
    clang-analyzer
    cmakelint
    clang-tools
    misc-nix.neovim
    compdb
  ];

  CXX = "${clang-analyzer}/libexec/scan-build/c++-analyzer";
  CC = "${clang-analyzer}/libexec/scan-build/ccc-analyzer";

  # CXX = "${gcc}/bin/g++";
  # CCC = "${gcc}/bin/gcc";
}
