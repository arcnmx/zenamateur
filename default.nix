{ pkgs ? import <nixpkgs> { } }: with pkgs; let
  arc = import (builtins.fetchTarball {
    url = "https://github.com/arcnmx/nixexprs/archive/master.tar.gz";
  }) { inherit pkgs; };
  libsmu = pkgs.libsmu or arc.pkgs.libsmu;

  smucmd = stdenv.mkDerivation {
    pname = "smucmd";
    version = "0";

    src = ./smucmd.c;

    buildInputs = [ libsmu ];

    unpackPhase = ":"; # doUnpack? skipUnpack?

    buildPhase = ''
      runHook preBuild

      $CC $src -lsmu -o $pname

      runHook postBuild
    '';

    installPhase = ''
      runHook preInstall

      install -Dm0755 -t $out/bin smucmd

      runHook postInstall
    '';
  };

  shell = mkShell {
    # ???
  };
in {
  inherit shell smucmd;
}
