{ ... }:
{
  perSystem =
    { self', pkgs, ... }:
    {
      devShells.repro-hook = pkgs.python3.pkgs.callPackage ./shell.nix {
        inherit (self'.packages) repro-hook;
      };
      packages = {
        repro-hook = pkgs.python3.pkgs.callPackage ./default.nix { };
      };
    };
}
