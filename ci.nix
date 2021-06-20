{ pkgs, lib, ... }: with lib; let
  zenamateur = import ./. { inherit pkgs; };
in {
  name = "zenamateur";
  ci.gh-actions.enable = true;
  cache.cachix.arc.publicKey = "arc.cachix.org-1:DZmhclLkB6UO0rc0rBzNpwFbbaeLfyn+fYccuAy7YVY=";
  channels.nixpkgs = "stable";

  tasks.smucmd = {
    name = "build smucmd";
    inputs = singleton zenamateur.smucmd;
  };
}
