import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import vinext from 'vinext';

export default defineConfig({
  plugins: [vinext(), react()],
});