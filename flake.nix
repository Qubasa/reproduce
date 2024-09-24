{
  description = "distributed data daemon for exchanging data (like hostnames) in distributed networks";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    flake-parts.inputs.nixpkgs-lib.follows = "nixpkgs";

    treefmt-nix.url = "github:numtide/treefmt-nix";
    treefmt-nix.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs =
    inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } (
      { lib, self, ... }:
      {
        imports = [
          ./treefmt.nix
          ./reproduce/flake-module.nix
        ];
        systems = [
          "aarch64-linux"
          "x86_64-linux"
        ];
      }
    );
}
