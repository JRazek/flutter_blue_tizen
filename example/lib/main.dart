import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  FlutterBlue flutterBlue = FlutterBlue.instance;
  flutterBlue.setLogLevel(LogLevel.error);
  flutterBlue.startScan(
    timeout: const Duration(seconds: 1),
    allowDuplicates: false,
  );

  // Listen to scan results
  var subscription = flutterBlue.scanResults.listen((results) {
    // do something with scan results
    if (results.isNotEmpty) {
      debugPrint(
          '${results.last.device.name} found! rssi: ${results.last.device.name}');
    }
  });

  // Stop scanning
  flutterBlue.stopScan();
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
