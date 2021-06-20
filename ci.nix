{ pkgs, lib, ... }: with lib; let
  zenamateur = import ./. { inherit pkgs; };
in {
  name = "zenamateur";
  ci.gh-actions.enable = true;
  cache.cachix.arc.enable = true;
  channels.nixpkgs = "stable";

  tasks.smucmd = {
    name = "build smucmd";
    inputs = singleton zenamateur.smucmd;
  };
}
