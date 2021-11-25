import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  FlutterBlue flutterBlue = FlutterBlue.instance;

  bool t = await flutterBlue.isOn;
  Future f = await flutterBlue.startScan();

  debugPrint("Here1232123212312312312312312");
  debugPrint(t.toString());
  // debugPrint("AVAILABLE " + flutterBlue.isAvailable.toString());
  runApp(const MyApp());
  // flutterBlue.setLogLevel(LogLevel.error);
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: const Center(
          child: Text('Running on: '),
        ),
      ),
    );
  }
}
