import dynamic from 'next/dynamic';
import Head from 'next/head';

// O editor depende fortemente do runtime Tauri (window/navigator e import dinâmico
// de '@tauri-apps/api'). Desabilitamos SSR para evitar mismatch e referência a APIs
// inexistentes no servidor durante `next build`/`next export`.
const EditorApp = dynamic(() => import('../src/App'), { ssr: false });

export default function HomePage() {
  return (
    <>
      <Head>
        <title>Lightning Engine Editor</title>
        <meta name="description" content="Editor Next.js da Lightning Engine" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
      </Head>
      <EditorApp />
    </>
  );
}