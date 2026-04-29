// @ts-check

const siteUrl = process.env.VERCEL_URL
  ? `https://${process.env.VERCEL_URL}`
  : 'http://localhost';

module.exports = async function createConfig() {
  const remarkMath = (await import('remark-math')).default;
  const rehypeKatex = (await import('rehype-katex')).default;

  /** @type {import('@docusaurus/types').Config} */
  const config = {
    title: 'Sistemas Operativos - Documentacion',
    tagline: 'Resolucion de practicos',
    url: siteUrl,
    baseUrl: '/',
    onBrokenLinks: 'warn',
    onBrokenMarkdownLinks: 'warn',
    i18n: {
      defaultLocale: 'es',
      locales: ['es']
    },
    presets: [
      [
        'classic',
        {
          docs: {
            routeBasePath: '/',
            sidebarPath: require.resolve('./sidebars.js'),
            remarkPlugins: [remarkMath],
            rehypePlugins: [rehypeKatex]
          },
          blog: false,
          theme: {
            customCss: require.resolve('./src/css/custom.css')
          }
        }
      ]
    ],
    stylesheets: [
      {
        href: 'https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css',
        type: 'text/css',
        integrity:
          'sha384-n8MVd4RsNIU0tAv4ct0nTaAbDJwPJzDEaqSD1odI+WdtXRGWt2kTvGFasHpSy3SV',
        crossorigin: 'anonymous'
      }
    ],
    themeConfig: {
      navbar: {
        title: 'Sistemas Operativos',
        items: [
          {
            type: 'docSidebar',
            sidebarId: 'tutorialSidebar',
            position: 'left',
            label: 'Documentacion'
          }
        ]
      }
    }
  };

  return config;
};
