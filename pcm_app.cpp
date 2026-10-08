//Grupo 7
//Nomes: Leonardo Campos, Matheus Henrique, Wisley Victor, Lucas Rodrigues

#include <gst/gst.h>
#include <iostream>
#include <map>
#include <string>
using namespace std;

struct PCM { int rate; string formato; int canais; };

static int rodar(const string &nome, const string &descricao) {
  cout << "[" << nome << "] " << descricao << "\n";

  GError *erro = nullptr;
  GstElement *pipeline = gst_parse_launch(descricao.c_str(), &erro);
  if (!pipeline || erro) {
    cerr << "[" << nome << "] ERRO ao montar pipeline: " << erro->message << "\n";
    return 1;
  }

  GstBus *bus = gst_element_get_bus(pipeline);
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  GstMessage *msg = gst_bus_timed_pop_filtered(
      bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  int status = 0;
  if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
    gst_message_parse_error(msg, &erro, nullptr);
    cerr << "[" << nome << "] ERRO: " << erro->message << "\n";
    status = 1;
  } else {
    cout << "[" << nome << "] concluido\n";
  }

  gst_message_unref(msg);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(bus);
  gst_object_unref(pipeline);
  return status;
}

int main(int argc, char *argv[]) {
  gst_init(&argc, &argv);

  // PCM = { taxa de amostragem, formato, canais}
  map<string, PCM> presets = {
      {"A", {44100, "S16LE", 2}},  // 44100 Hz, 16 bits, estereo
      {"B", {8000, "U8", 1}},      // 8000 Hz, 8 bits, mono
      {"C", {22050, "S16LE", 1}},  // 22050 Hz, 16 bits, mono
  };

  string nome;
  PCM cfg;
  double freq = 440;   // frequencia do tom gerado (Hz); default 440, trocavel no ultimo argumento
  bool modoValido = false;

  if (argc >= 2) {
    string primeiroArg = argv[1];

    if (presets.count(primeiroArg)) {
      // ./pcm_app A|B|C [freq]
      nome = primeiroArg;
      cfg = presets[nome];
      if (argc == 3) freq = atof(argv[2]);
      modoValido = (argc == 2 || argc == 3);
    } else if (primeiroArg == "custom" && (argc == 5 || argc == 6)) {
      // ./pcm_app custom <rate> <formato> <canais> [freq]
      cfg = {atoi(argv[2]), argv[3], atoi(argv[4])};
      nome = string("custom_") + argv[2] + "_" + argv[3] + "_" + argv[4];
      if (argc == 6) freq = atof(argv[5]);
      modoValido = true;
    }
  }

  if (!modoValido) {
    cerr << "Uso: " << argv[0] << " A|B|C [freq]   ou   " << argv[0]
         << " custom <rate> <formato> <canais> [freq]\n";
    return 1;
  }

  g_mkdir_with_parents("saida", 0755);

  string video =
      "videotestsrc num-buffers=150 ! video/x-raw,width=640,height=480,framerate=30/1 "
      "! videoconvert ! video/x-raw,format=I420 ! x264enc tune=zerolatency bitrate=1000 "
      "! h264parse ! mp4mux ! filesink location=saida/video.mp4";

  string audio =
      "audiotestsrc wave=saw num-buffers=215 samplesperbuffer=1024 freq=" + to_string(freq) + " volume=0.8 "
      "! audio/x-raw,format=S16LE,rate=44100,channels=2 "
      "! audioconvert ! audioresample "
      "! audio/x-raw,format=" + cfg.formato + ",rate=" + to_string(cfg.rate) +
      ",channels=" + to_string(cfg.canais) +
      " ! wavenc ! filesink location=saida/audio_" + nome + ".wav";

  if (rodar("VIDEO", video) != 0) return 1;
  if (rodar("AUDIO", audio) != 0) return 1;

  cout << "OK! Arquivos em ./saida\n";
  return 0;
}