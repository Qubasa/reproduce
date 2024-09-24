{ ... }:
{
  perSystem =
    { self', pkgs, ... }:
    {
      devShells.reproduce = pkgs.python3.pkgs.callPackage ./shell.nix {
        inherit (self'.packages) reproduce;
      };
      packages = rec {
        default = reproduce;
        reproduce = pkgs.python3.pkgs.callPackage ./default.nix { };
      };
    };
}
