/** @type {import('next').NextConfig} */
const nextConfig = {
  // Static export gera /out, consumido pelo Tauri (frontendDist).
  output: 'export',
  reactStrictMode: true,
  images: { unoptimized: true },
  // Tauri serve via protocolo customizado; trailing slash facilita roteamento estático.
  trailingSlash: true,
};

module.exports = nextConfig;
