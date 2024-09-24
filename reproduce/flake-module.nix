{ ... }:
{
  perSystem =
    { self', pkgs, ... }:
    {
      devShells.reproduce = pkgs.python3.pkgs.callPackage ./shell.nix {
        inherit (self'.packages) reproduce;
      };
      packages = {
        reproduce = pkgs.python3.pkgs.callPackage ./default.nix { };
      };
    };
}
