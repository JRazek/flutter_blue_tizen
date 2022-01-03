import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

class BluetoothHelper {}

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  FlutterBlue flutterBlue = FlutterBlue.instance;
  // flutterBlue.setLogLevel(LogLevel.alert);
  flutterBlue.startScan();

  var subscription = flutterBlue.scanResults.listen((results) async {
    // do something with scan results
    if (results.isNotEmpty) {
      BluetoothDevice dev = results.last.device;
      debugPrint('${dev.name} found! rssi: ${dev.name}');
      dev.connect();
      // debugPrint('connect done');
    }
  });

  // Stop scanning
  // flutterBlue.stopScan();
  runApp(const MyApp());
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
