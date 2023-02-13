//
//  ViewController.m
//  RE2Sample
//
//  Created by Yi Wang on 2/12/23.
//

#import "ViewController.h"

void test_tokenizer(const char* merges_path, const char* vocab_path);

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  NSString* merges = [[NSBundle mainBundle] pathForResource:@"merges"
						   ofType:@"txt"];
  NSString* vocab = [[NSBundle mainBundle] pathForResource:@"vocab"
						   ofType:@"txt"];
  test_tokenizer([merges UTF8String], [vocab UTF8String]);
}


@end
